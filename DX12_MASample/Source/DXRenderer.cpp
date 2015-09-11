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
#include "DXRenderer.h"

using Microsoft::WRL::ComPtr;

DXRenderer::DXRenderer(std::vector<DXDevice*> devices, ComPtr<IDXGIFactory4> dxgiFactory, UINT width, UINT height, HWND hwnd):
	mDXDevices(devices),
	mViewport(),
	mScissorRect(),
	mDXGIFactory(dxgiFactory)
{
	// Set window properties
	mWndProp.width = width;
	mWndProp.height = height;
	mWndProp.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	mWndProp.hwnd = hwnd;

	// Setup viewport and scissor rect 
	mViewport.Width = static_cast<float>(width);
	mViewport.Height = static_cast<float>(height);
	mViewport.MaxDepth = 1.0f;
	mScissorRect.right = static_cast<LONG>(width);
	mScissorRect.bottom = static_cast<LONG>(height);
}

DXRenderer::~DXRenderer()
{}

void DXRenderer::OnRender()
{
	mCurrentFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
	
	PopulateCommandLists(); // Record all the commands we want to perform
	ExecuteCommandLists();

	ThrowIfFailed(mSwapChain->Present(0, 0));
	MoveToNextFrame();
}

void DXRenderer::OnDestroy()
{
	// Wait for GPU to be done
	MoveToNextFrame();

	CloseHandle(mFenceEvent);
}

void DXRenderer::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

void DXRenderer::UpdateConstantBuffer(ComPtr<ID3D12Resource> constantBuffer, ConstantBuffer* constantBufferData, const UINT dataSize)
{
	if (constantBuffer == NULL)
		return;
	UINT8* cbvDataBegin = nullptr;
	ThrowIfFailed(constantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&cbvDataBegin)));
	memcpy(cbvDataBegin, constantBufferData, dataSize);
	constantBuffer->Unmap(0, nullptr);
}

void DXRenderer::UpdateVertexBuffer(ComPtr<ID3D12Resource> vertexBuffer, Vertex* vertexData, const UINT vertexDataSize)
{
	if (vertexBuffer == NULL)
		return;
	// Copy vertices into vertex buffer
	UINT8* dataBegin = nullptr;
	ThrowIfFailed(vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&dataBegin)));
	memcpy(dataBegin, vertexData, vertexDataSize);
	vertexBuffer->Unmap(0, nullptr);
}

void DXRenderer::UpdateTextureData(ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12Resource> texture, ComPtr<ID3D12Resource> textureUploadHeap, void* textureData, UINT pixelSize)
{
	commandList.Get()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

	D3D12_RESOURCE_DESC textureDesc = texture->GetDesc();
	UINT64 width = textureDesc.Width;
	UINT height = textureDesc.Height;

	// Copy data to the intermediate upload heap and then schedule a copy 
	// from the upload heap to the Texture2D.
	D3D12_SUBRESOURCE_DATA textureSubresourceData = {};
	textureSubresourceData.pData = textureData;
	textureSubresourceData.RowPitch = width * pixelSize; 
	textureSubresourceData.SlicePitch = textureSubresourceData.RowPitch * height;
	UpdateSubresources(commandList.Get(), texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureSubresourceData);

	commandList.Get()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void DXRenderer::CreateTexture(DXDevice* device, D3D12_RESOURCE_DESC textureDesc, ComPtr<ID3D12Resource>* texture, ComPtr<ID3D12Resource>* textureUploadHeap, const UINT heapIndex)
{
	// Describe and create a Texture2D.
	textureDesc.MipLevels = 1;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	ThrowIfFailed(device->mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&(*texture))));


	if (textureUploadHeap != nullptr) // Only create upload heap if CPU uploads to texture
	{
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize((*texture).Get(), 0, 1);
		// Create the GPU upload buffer.
		ThrowIfFailed(device->mDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&(*textureUploadHeap))));
	}

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// Add SRV to descriptor heap 
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(device->mCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
	srvHandle.Offset(device->mCbvSrvUavDescriptorSize * heapIndex);
	device->mDevice->CreateShaderResourceView((*texture).Get(), &srvDesc, srvHandle);
}

void DXRenderer::CreatePipelineState(DXDevice* dxDevice, ComPtr<ID3D12PipelineState>* pipelineStateObject, D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc, std::wstring shaderPath, D3D12_INPUT_ELEMENT_DESC inputLayoutDesc[], UINT inputLayoutCount, ComPtr<ID3D12RootSignature> rootSignature)
{

#if DEBUG
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	// Reference to PSO
	ID3D12PipelineState* pso = (*pipelineStateObject).Get();

	//// Get extension
	//std::wstring::size_type idx = shaderPath.rfind('.');
	//std::wstring extension;
	//if (idx != std::wstring::npos)
	//{
	//	extension = shaderPath.substr(idx + 1);
	//}

	ComPtr<ID3DBlob> vertexShader, pixelShader, errorBlob;
	if (shaderPath.compare(L""))
	{
		// Compile the shaders
		HRESULT hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "VShader", "vs_5_0", compileFlags, 0, vertexShader.GetAddressOf(), errorBlob.GetAddressOf());
		if (!SUCCEEDED(hr))
		{
			const char* errMessage = (const char*)errorBlob.Get()->GetBufferPointer();
			printf("%s", errMessage);
		}
		hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "PShader", "ps_5_0", compileFlags, 0, pixelShader.GetAddressOf(), errorBlob.GetAddressOf());
		if (!SUCCEEDED(hr))
		{
			const char* errMessage = (const char*)errorBlob.Get()->GetBufferPointer();
			printf("%s", errMessage);
		}
		psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
		psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
	}

	// Describe PSO
	psoDesc.InputLayout = { inputLayoutDesc, inputLayoutCount };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	// Create PSO
	ThrowIfFailed(dxDevice->mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&(*pipelineStateObject))));
}

void DXRenderer::CreateVertexBuffer(DXDevice* dxDevice, D3D12_VERTEX_BUFFER_VIEW* vertexBufferView, MS::ComPtr<ID3D12Resource>* vertexBuffer, Vertex* vertexData, const UINT vertexDataSize, const UINT vertexSize)
{
	//UINT vertexDataSize = vertexData.size() * vertexSize;
	// Create resource on device for vertex buffer
	ThrowIfFailed(dxDevice->mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexDataSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&(*vertexBuffer))));

	// Copy vertices into vertex buffer
	UINT8* dataBegin = nullptr;
	ThrowIfFailed((*vertexBuffer)->Map(0, nullptr, reinterpret_cast<void**>(&dataBegin)));
	memcpy(dataBegin, vertexData, vertexDataSize);
	(*vertexBuffer)->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	vertexBufferView->BufferLocation = (*vertexBuffer).Get()->GetGPUVirtualAddress();
	vertexBufferView->StrideInBytes = vertexSize;
	vertexBufferView->SizeInBytes = vertexDataSize;
}

void DXRenderer::CreateConstantBuffer(DXDevice* dxDevice, ComPtr<ID3D12Resource>* constantBuffer, ConstantBuffer* constantBufferData, const UINT constantBufferSize, const UINT cbvHeapPosition)
{
	// Create resource for constant buffer
	ThrowIfFailed(dxDevice->mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&(*constantBuffer))));

	// Describe and create a constant buffer view
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = (*constantBuffer)->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (constantBufferSize + 255) & ~255; // Require size to be 256 byte aligned
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(dxDevice->mCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
	cbvHandle.Offset(dxDevice->mCbvSrvUavDescriptorSize * cbvHeapPosition);
	dxDevice->mDevice->CreateConstantBufferView(&cbvDesc, cbvHandle);

	// Copy constant buffer data into constant buffer
	UINT8* cbvDataBegin;
	ThrowIfFailed((*constantBuffer)->Map(0, nullptr, reinterpret_cast<void**>(&cbvDataBegin)));
	memcpy(cbvDataBegin, &constantBufferData, constantBufferSize);
	(*constantBuffer)->Unmap(0, nullptr);
}

void DXRenderer::CreateSwapChain(DXDevice* dxDevice, DXGI_SWAP_CHAIN_DESC1 swapChainDesc)
{
	// Invariant SwapChain properties
	swapChainDesc.Width = mWndProp.width;
	swapChainDesc.Height = mWndProp.height;
	swapChainDesc.BufferCount = NumRenderTargets; // Required because of FLIP SwapEffect

	// Create SwapChain
	IDXGISwapChain1* tempSwapChain1 = nullptr; // Call to CreateSwapChainForHwnd() requires a SwapChain1 object
	ThrowIfFailed(mDXGIFactory->CreateSwapChainForHwnd(dxDevice->mCommandQueue.Get(), mWndProp.hwnd, &swapChainDesc, nullptr, nullptr, &tempSwapChain1)); 
	ThrowIfFailed(tempSwapChain1->QueryInterface(IID_PPV_ARGS(&(mSwapChain)))); // Copy the temp SwapChain1 into our desired SwapChain3
	tempSwapChain1->Release();

	// Get a handle to the memory location (CPU) of the descriptor heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(dxDevice->mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	dxDevice->mRtvDescriptorSize = dxDevice->mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create render target and view for each swap chain buffer
	for (UINT s = 0; s < swapChainDesc.BufferCount; s++) {
		// Get buffer render target
		ThrowIfFailed(mSwapChain->GetBuffer(s, IID_PPV_ARGS(&dxDevice->mRenderTargets[s])));
		// Create render target view
		dxDevice->mDevice->CreateRenderTargetView(dxDevice->mRenderTargets[s].Get(), nullptr, rtvHandle);
		// Increment the RTV heap handle
		rtvHandle.Offset(1, dxDevice->mRtvDescriptorSize); 
	}
}
