/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "PrecompiledHeaders.h" 
#include <array>
#include "DXDevice.h"
#include "DXUtil.h"

enum CrossAdapterProperties
{
	NumDevices = 2
};

class DXCrossAdapterResources
{
public:
	// Command list and allocator for copy operation on primary GPU 
	MS::ComPtr<ID3D12CommandAllocator> mCopyCommandAllocator;
	MS::ComPtr<ID3D12CommandQueue> mCopyCommandQueue;
	MS::ComPtr<ID3D12GraphicsCommandList> mCopyCommandList;
	// Cross-adapter synchronization 
	std::array<MS::ComPtr<ID3D12Fence>, NumDevices> mFences;
	std::array<HANDLE, NumDevices> mFenceEvents;
	std::array<UINT64, NumRenderTargets> mFenceValues;
	UINT64 mCurrentFenceValue;

	DXCrossAdapterResources(DXDevice* primaryDevice, DXDevice* secondaryDevice);
	void CreateResources();
	void CreateCommandList();
	void PopulateCommandList(int currentFrameIndex);
	void SetupFences();

protected:
	// References to devices
	DXDevice* mPrimaryDevice;
	DXDevice* mSecondaryDevice;
	// Cross-adapter resources
	std::array<MS::ComPtr<ID3D12Resource>, NumRenderTargets> mShaderResources;
	std::array<MS::ComPtr<ID3D12Resource>, NumRenderTargets> mShaderResourceViews;

};
