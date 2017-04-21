/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

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
