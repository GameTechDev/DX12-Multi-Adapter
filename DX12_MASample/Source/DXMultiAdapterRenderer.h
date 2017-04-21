/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

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
