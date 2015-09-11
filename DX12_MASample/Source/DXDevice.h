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

#pragma once
#include "PrecompiledHeaders.h"
#include "DXUtil.h"

class DXDevice
{
public:
	MS::ComPtr<ID3D12Device> mDevice;
	MS::ComPtr<ID3D12CommandQueue> mCommandQueue;
	MS::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	std::array<MS::ComPtr<ID3D12Resource>, NumRenderTargets> mRenderTargets;

	MS::ComPtr<ID3D12Fence> mFence;
	std::array<UINT64, NumRenderTargets> mFenceValues;
	UINT64 mCurrentFenceValue;
	HANDLE mFenceEvent;

	MS::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	UINT mRtvDescriptorSize;
	MS::ComPtr<ID3D12DescriptorHeap> mCbvSrvUavHeap;
	UINT mCbvSrvUavDescriptorSize;

	DXDevice(IDXGIAdapter* adapter);
	~DXDevice();
	void CreateDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc, D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc);	
};
