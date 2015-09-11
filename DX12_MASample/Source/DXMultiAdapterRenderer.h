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
#include "DXRenderer.h"
#include "DXCrossAdapterResources.h"
#include "DXUtil.h"

class DXMultiAdapterRenderer : public DXRenderer
{
public:
	DXMultiAdapterRenderer(std::vector<DXDevice*> devices, MS::ComPtr<IDXGIFactory4> dxgiFactory, UINT width, UINT height, HWND hwnd);
	virtual void OnUpdate() override;
	float GetSharePercentage();
	void IncrementSharePercentage();
	void DecrementSharePercentage();

protected:
	float mSharePercentage = 0.5f;

	DXCrossAdapterResources* mCrossAdapterResources;

	MS::ComPtr<ID3D12Resource> mQuadTopVertexBuffer;
	MS::ComPtr<ID3D12Resource> mQuadBottomVertexBuffer;
	MS::ComPtr<ID3D12Resource> mQuadCombineTopVertexBuffer;
	MS::ComPtr<ID3D12Resource> mQuadCombineBottomVertexBuffer;

	ConstantBufferTime mConstantBufferTimePrimaryData;
	MS::ComPtr<ID3D12Resource> mConstantBufferTimePrimary;
	D3D12_GPU_DESCRIPTOR_HANDLE mTimePrimaryCbvHandle;

	ConstantBufferTime mConstantBufferTimeSecondaryData;
	MS::ComPtr<ID3D12Resource> mConstantBufferTimeSecondary;
	D3D12_GPU_DESCRIPTOR_HANDLE mTimeSecondaryCbvHandle;

	ConstantBufferTransform mConstantBufferTransformTopData;
	MS::ComPtr<ID3D12Resource> mConstantBufferTransformTop;
	D3D12_GPU_DESCRIPTOR_HANDLE mTransformTopCbvHandle;

	ConstantBufferTransform mConstantBufferTransformBottomData;
	MS::ComPtr<ID3D12Resource> mConstantBufferTransformBottom;
	D3D12_GPU_DESCRIPTOR_HANDLE mTransformBottomCbvHandle;

	MS::ComPtr<ID3D12Resource> mTexture;
	D3D12_GPU_DESCRIPTOR_HANDLE mTextureSrvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE mTextureRtvHandle;

	virtual void CreateRootSignatures() override;
	virtual void LoadPipeline() override;
	virtual void LoadAssets() override;
	virtual void CreateCommandLists() override;
	virtual void PopulateCommandLists() override;
	virtual void ExecuteCommandLists() override;
	virtual void MoveToNextFrame() override;
};
