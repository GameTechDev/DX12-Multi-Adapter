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
