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
