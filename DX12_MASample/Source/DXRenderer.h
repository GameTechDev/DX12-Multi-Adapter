/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <vector>
#include "DXDevice.h"
#include "DXUtil.h"

struct ConstantBuffer {};
struct Vertex {};

struct VertexColor : Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

struct VertexUV : Vertex
{
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT2 uv;
};

struct ConstantBufferOffset : ConstantBuffer
{
	DirectX::XMFLOAT4 offset;
};

struct ConstantBufferTransform : ConstantBuffer
{
	DirectX::XMMATRIX modelViewProjection;
};

struct ConstantBufferTime : ConstantBuffer
{
	float time;
};

struct ConstantBufferDepthImage : ConstantBuffer
{
	DirectX::XMMATRIX modelViewProjection;
};

class DXRenderer
{
public:
	DXRenderer(std::vector<DXDevice*> devices, MS::ComPtr<IDXGIFactory4> dxgiFactory, UINT width, UINT height, HWND hwnd);
	virtual ~DXRenderer();
	void OnRender();
	void OnDestroy();

	virtual void OnInit();
	virtual void OnUpdate() = 0;

protected:
	WindowProperties mWndProp;
	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
	MS::ComPtr<IDXGIFactory4> mDXGIFactory;
	MS::ComPtr<IDXGISwapChain3> mSwapChain;
	int mCurrentFrameIndex;

	std::vector<DXDevice*> mDXDevices;
	std::vector<MS::ComPtr<ID3D12GraphicsCommandList>> mCommandLists;
	std::vector<MS::ComPtr<ID3D12PipelineState>> mPipelineStates;
	std::vector<MS::ComPtr<ID3D12RootSignature>> mRootSignatures;
	std::vector<D3D12_VERTEX_BUFFER_VIEW*> mVertexBufferViews;

	UINT64 mCurrentFenceValue;
	UINT64 mFenceValues[NumRenderTargets];
	HANDLE mFenceEvent;

	void UpdateConstantBuffer(MS::ComPtr<ID3D12Resource> constantBuffer, ConstantBuffer* constantBufferData, const UINT dataSize);
	void UpdateVertexBuffer(MS::ComPtr<ID3D12Resource> vertexBuffer, Vertex* vertexBufferData, const UINT dataSize);
	void UpdateTextureData(MS::ComPtr<ID3D12GraphicsCommandList> commandList, MS::ComPtr<ID3D12Resource> texture, MS::ComPtr<ID3D12Resource> textureUploadHeap, void* textureData, UINT pixelSize);

	void CreateTexture(DXDevice* device, D3D12_RESOURCE_DESC textureDesc, MS::ComPtr<ID3D12Resource>* texture, MS::ComPtr<ID3D12Resource>* textureUploadHeap, const UINT heapIndex);
	void CreatePipelineState(DXDevice* dxDevice, MS::ComPtr<ID3D12PipelineState>* pipelineStateObject, D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc, std::wstring shaderPath, D3D12_INPUT_ELEMENT_DESC inputLayoutDesc[], UINT inputLayoutCount, MS::ComPtr<ID3D12RootSignature> rootSignature);
	void CreateVertexBuffer(DXDevice* dxDevice, D3D12_VERTEX_BUFFER_VIEW* vertexBufferView, MS::ComPtr<ID3D12Resource>* vertexBuffer, Vertex* vertexData, const UINT vertexDataSize, const UINT vertexSize);
	void CreateConstantBuffer(DXDevice* dxDevice, MS::ComPtr<ID3D12Resource>* constantBuffer, ConstantBuffer* constantBufferData, const UINT constantBufferSize, const UINT cbvHeapPosition);
	void CreateSwapChain(DXDevice* dxDevice, DXGI_SWAP_CHAIN_DESC1 swapChainDesc);

	virtual void CreateRootSignatures() = 0;
	virtual void LoadPipeline() = 0;
	virtual void LoadAssets() = 0;
	virtual void CreateCommandLists() = 0;
	virtual void PopulateCommandLists() = 0;
	virtual void ExecuteCommandLists() = 0;
	virtual void MoveToNextFrame() = 0;
};
