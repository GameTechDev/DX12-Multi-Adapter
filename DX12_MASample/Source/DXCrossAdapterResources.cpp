//--------------------------------------------------------------------------------------
// Copyright 2015 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------

#include "PrecompiledHeaders.h"
#include "DXCrossAdapterResources.h"

DXCrossAdapterResources::DXCrossAdapterResources(DXDevice * primaryDevice, DXDevice * secondaryDevice) :
	mPrimaryDevice(primaryDevice),
	mSecondaryDevice(secondaryDevice)
{
	// Create copy command allocator on primary device
	ThrowIfFailed(mPrimaryDevice->mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCopyCommandAllocator)));

	// Create copy command queue on primary device
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed(mPrimaryDevice->mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCopyCommandQueue)));
}

void DXCrossAdapterResources::CreateResources()
{
	// Describe cross-adapter shared resources on primaryDevice adapter
	D3D12_RESOURCE_DESC crossAdapterDesc = mPrimaryDevice->mRenderTargets[0]->GetDesc();
	crossAdapterDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
	crossAdapterDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// Create a shader resource and shared handle for each buffer
	for (int i = 0; i < NumRenderTargets; i++)
	{
		ThrowIfFailed(mPrimaryDevice->mDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER,
			&crossAdapterDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(&mShaderResources[i])));

		HANDLE heapHandle = nullptr;
		ThrowIfFailed(mPrimaryDevice->mDevice->CreateSharedHandle(
			mShaderResources[i].Get(),
			nullptr,
			GENERIC_ALL,
			nullptr,
			&heapHandle));

		// Open shared handle on secondaryDevice device
		ThrowIfFailed(mSecondaryDevice->mDevice->OpenSharedHandle(heapHandle, IID_PPV_ARGS(&mShaderResourceViews[i])));

		CloseHandle(heapHandle);
	}

	// Create a shader resource view (SRV) for each of the cross adapter resources
	CD3DX12_CPU_DESCRIPTOR_HANDLE secondarySRVHandle(mSecondaryDevice->mCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < NumRenderTargets; i++)
	{
		mSecondaryDevice->mDevice->CreateShaderResourceView(mShaderResourceViews[i].Get(), nullptr, secondarySRVHandle);
		secondarySRVHandle.Offset(mSecondaryDevice->mCbvSrvUavDescriptorSize);
	}

	// Create fence for cross adapter resources
	ThrowIfFailed(mPrimaryDevice->mDevice->CreateFence(mCurrentFenceValue,
		D3D12_FENCE_FLAG_SHARED | D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER,
		IID_PPV_ARGS(&mFences[Device_Primary])));

	// Create a shared handle to the cross adapter fence
	HANDLE fenceHandle = nullptr;
	ThrowIfFailed(mPrimaryDevice->mDevice->CreateSharedHandle(
		mFences[Device_Primary].Get(),
		nullptr,
		GENERIC_ALL,
		nullptr,
		&fenceHandle));

	// Open shared handle to fence on secondaryDevice GPU
	ThrowIfFailed(mSecondaryDevice->mDevice->OpenSharedHandle(fenceHandle, IID_PPV_ARGS(&mFences[Device_Secondary])));
}

void DXCrossAdapterResources::CreateCommandList()
{
	// Create copy command list on primary device
	ThrowIfFailed(mPrimaryDevice->mDevice->CreateCommandList(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		mCopyCommandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&mCopyCommandList)));

	ThrowIfFailed(mCopyCommandList->Close()); // Releases the copy allocator
}

void DXCrossAdapterResources::PopulateCommandList(int currentFrameIndex)
{
	// Create the copy command list for the primaryDevice adapter
	ThrowIfFailed(mCopyCommandAllocator->Reset());
	ThrowIfFailed(mCopyCommandList->Reset(mCopyCommandAllocator.Get(), nullptr));

	// Transition cross adapter resources to be a copy destination
	mCopyCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShaderResources[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

	// Copy from primaryDevice render targets into cross adapter resources
	mCopyCommandList->CopyResource(mShaderResources[currentFrameIndex].Get(), mPrimaryDevice->mRenderTargets[currentFrameIndex].Get());

	// Transition cross adapter resources back to being a pixel shader resource
	mCopyCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShaderResources[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	ThrowIfFailed(mCopyCommandList->Close());
}

void DXCrossAdapterResources::SetupFences()
{
	// Setup fence for Primary GPU and wait for setup to finish
	{
		mFenceEvents[Device_Primary] = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		if (mFenceEvents[Device_Primary] == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
		// Schedule a signal in the command queue
		ThrowIfFailed(mPrimaryDevice->mCommandQueue->Signal(mFences[Device_Primary].Get(), mCurrentFenceValue));
		mFenceValues[0] = mCurrentFenceValue;
		mCurrentFenceValue++;

		// Wait until fence has been processed
		ThrowIfFailed(mFences[Device_Primary]->SetEventOnCompletion(mFenceValues[0], mFenceEvents[Device_Primary]));
		WaitForSingleObjectEx(mFenceEvents[Device_Primary], INFINITE, FALSE);
	}

	// Setup fence for Secondary GPU and wait for setup to finish
	{
		mFenceEvents[Device_Secondary] = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		if (mFenceEvents[Device_Secondary] == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
		// Schedule a signal in the command queue
		ThrowIfFailed(mSecondaryDevice->mCommandQueue->Signal(mFences[Device_Secondary].Get(), mCurrentFenceValue));
		mFenceValues[0] = mCurrentFenceValue;
		mCurrentFenceValue++;

		// Wait until fence has been processed
		ThrowIfFailed(mFences[Device_Secondary]->SetEventOnCompletion(mFenceValues[0], mFenceEvents[Device_Secondary]));
		WaitForSingleObjectEx(mFenceEvents[Device_Secondary], INFINITE, FALSE);
	}
}

