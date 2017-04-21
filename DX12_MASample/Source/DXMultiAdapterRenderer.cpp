/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "PrecompiledHeaders.h"
#include "DXMultiAdapterRenderer.h"

enum CommandList
{
	Primary_CommandList_Scene,
	Secondary_CommandList_Scene,
	Secondary_CommandList_Combine_Scene,
	CommandList_Count
};

enum RootSignature
{
	Primary_RootSignature_Scene,
	Secondary_RootSignature_CrossAdapter,
	Secondary_RootSignature_Scene,
	RootSignature_Count
};

enum PipelineState
{
	Primary_PipelineState_Scene,
	Secondary_PipelineState_CrossAdapter,
	Secondary_PipelineState_Scene,
	PipelineState_Count
};

enum PrimaryDescriptorList
{
	Primary_CBV_Time,
	Primary_Descriptor_Count
};

enum SecondaryDescriptorList
{
	Secondary_SRV_CA1,
	Secondary_SRV_CA2,
	Secondary_SRV_CA3,
	Secondary_CBV_Time,
	Secondary_CBV_Transform_Bottom,
	Secondary_CBV_Transform_Top,
	Secondary_SRV_Texture,
	Secondary_Descriptor_Count
};

enum VertexBuffer
{
	Primary_Quad_Top,
	Secondary_Quad_Bottom,
	Secondary_Quad_Combine_Top,
	Secondary_Quad_Combine_Bottom,
	VertexBuffer_Count
};

using Microsoft::WRL::ComPtr;

DXMultiAdapterRenderer::DXMultiAdapterRenderer(std::vector<DXDevice*> devices, MS::ComPtr<IDXGIFactory4> dxgiFactory, UINT width, UINT height, HWND hwnd) :
	DXRenderer(devices, dxgiFactory, width, height, hwnd)
{}

void DXMultiAdapterRenderer::OnUpdate()
{

	// Update vertex data for quads to adjust percentage of scene rendered by each GPU
	VertexUV quadVerticesTop[4];
	quadVerticesTop[0].position = { -1.0f, -1.0f * (mSharePercentage * 2 - 1), 0.0f, 1.0f };
	quadVerticesTop[0].uv = { 0.0f, static_cast<float>(1.0 - mSharePercentage) };
	quadVerticesTop[1].position = { -1.0f, 1.0f, 0.0f, 1.0f };
	quadVerticesTop[1].uv = { 0.0f, 1.0f };
	quadVerticesTop[2].position = { 1.0f, -1.0f * (mSharePercentage * 2 - 1), 0.0f, 1.0f };
	quadVerticesTop[2].uv = { 1.0f, static_cast<float>(1.0 - mSharePercentage) };
	quadVerticesTop[3].position = { 1.0f, 1.0f, 0.0f, 1.0f };
	quadVerticesTop[3].uv = { 1.0f, 1.0f };;

	UpdateVertexBuffer(mQuadTopVertexBuffer, quadVerticesTop, sizeof(quadVerticesTop));

	VertexUV quadVerticesBottom[4];
	quadVerticesBottom[0].position = { -1.0f, -1.0f, 0.0f, 1.0f };
	quadVerticesBottom[0].uv = { 0.0f, 0.0f };
	quadVerticesBottom[1].position = { -1.0f, -1.0f * (mSharePercentage * 2 - 1), 0.0f, 1.0f };
	quadVerticesBottom[1].uv = { 0.0f, static_cast<float>(1.0 - mSharePercentage) };
	quadVerticesBottom[2].position = { 1.0f, -1.0f, 0.0f, 1.0f };
	quadVerticesBottom[2].uv = { 1.0f, 0.0f };
	quadVerticesBottom[3].position = { 1.0f, -1.0f * (mSharePercentage * 2 - 1), 0.0f, 1.0f };
	quadVerticesBottom[3].uv = { 1.0f, static_cast<float>(1.0 - mSharePercentage) };;

	UpdateVertexBuffer(mQuadBottomVertexBuffer, quadVerticesBottom, sizeof(quadVerticesBottom));

	VertexUV quadVerticesCombineTop[4];
	quadVerticesCombineTop[0].position = { -1.0f, -1.0f, 0.0f, 1.0f };
	quadVerticesCombineTop[0].uv = { 0.0f, static_cast<float>(1.0 - mSharePercentage) };
	quadVerticesCombineTop[1].position = { -1.0f, 1.0f, 0.0f, 1.0f };
	quadVerticesCombineTop[1].uv = { 0.0f, 1.0f };
	quadVerticesCombineTop[2].position = { 1.0f, -1.0f, 0.0f, 1.0f };
	quadVerticesCombineTop[2].uv = { 1.0f, static_cast<float>(1.0 - mSharePercentage) };
	quadVerticesCombineTop[3].position = { 1.0f, 1.0f, 0.0f, 1.0f };
	quadVerticesCombineTop[3].uv = { 1.0f, 1.0f };

	UpdateVertexBuffer(mQuadCombineTopVertexBuffer, quadVerticesCombineTop, sizeof(quadVerticesCombineTop));

	VertexUV quadVerticesCombineBottom[4];
	quadVerticesCombineBottom[0].position = { -1.0f, -1.0f, 0.0f, 1.0f };
	quadVerticesCombineBottom[0].uv = { 0.0f, 0.0f };
	quadVerticesCombineBottom[1].position = { -1.0f, 1.0f, 0.0f, 1.0f };
	quadVerticesCombineBottom[1].uv = { 0.0f, static_cast<float>(1.0 - mSharePercentage) };
	quadVerticesCombineBottom[2].position = { 1.0f, -1.0f, 0.0f, 1.0f };
	quadVerticesCombineBottom[2].uv = { 1.0f, 0.0f };
	quadVerticesCombineBottom[3].position = { 1.0f, 1.0f, 0.0f, 1.0f };
	quadVerticesCombineBottom[3].uv = { 1.0f, static_cast<float>(1.0 - mSharePercentage) };

	UpdateVertexBuffer(mQuadCombineBottomVertexBuffer, quadVerticesCombineBottom, sizeof(quadVerticesCombineBottom));

	// Update constant buffers with time data
	const float translationSpeed = 0.02f;

	// Move offset indices along x axis 
	mConstantBufferTimePrimaryData.time += translationSpeed;
	mConstantBufferTimeSecondaryData.time += translationSpeed;

	UpdateConstantBuffer(mConstantBufferTimePrimary, &mConstantBufferTimePrimaryData, sizeof(mConstantBufferTimePrimaryData));
	UpdateConstantBuffer(mConstantBufferTimeSecondary, &mConstantBufferTimeSecondaryData, sizeof(mConstantBufferTimeSecondaryData));
}

float DXMultiAdapterRenderer::GetSharePercentage()
{
	return mSharePercentage;
}

void DXMultiAdapterRenderer::IncrementSharePercentage()
{
	if (mSharePercentage < 1.0f)
		mSharePercentage += 0.05f;
	return;
}

void DXMultiAdapterRenderer::DecrementSharePercentage()
{
	if (mSharePercentage > 0.0f)
		mSharePercentage -= 0.05f;
	return;
}

void DXMultiAdapterRenderer::LoadPipeline()
{
	// Setup primary GPU 
	{
		// Describe descriptor heaps - RTV and CBV/SRV/UAV
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = NumRenderTargets;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
		cbvSrvUavHeapDesc.NumDescriptors = Primary_Descriptor_Count;

		mDXDevices[Device_Primary]->CreateDescriptorHeaps(rtvHeapDesc, cbvSrvUavHeapDesc);

		// Clear value used for render target background
		D3D12_CLEAR_VALUE clearValue = {};
		const float clearColor[4] = { 0.0f, 0.2f, 0.3f, 1.0f };
		memcpy(clearValue.Color, clearColor, sizeof(clearColor));
		clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		CD3DX12_CPU_DESCRIPTOR_HANDLE primaryRTVHandle(mDXDevices[Device_Primary]->mRtvHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT s = 0; s < NumRenderTargets; s++) // One render target resource per buffer
		{
			ThrowIfFailed(mDXDevices[Device_Primary]->mDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Tex2D(
					DXGI_FORMAT_R8G8B8A8_UNORM, // Same as swap chain format
					mWndProp.width,
					mWndProp.height,
					1u, 1u,
					1, // Sampler count
					0, // Sampler quality level
					D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
					D3D12_TEXTURE_LAYOUT_UNKNOWN, 0u),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				&clearValue,
				IID_PPV_ARGS(&mDXDevices[Device_Primary]->mRenderTargets[s])));
			mDXDevices[Device_Primary]->mDevice->CreateRenderTargetView(mDXDevices[Device_Primary]->mRenderTargets[s].Get(), nullptr, primaryRTVHandle); // Create RTV in heap
			primaryRTVHandle.Offset(1 * mDXDevices[Device_Primary]->mRtvDescriptorSize); // Step through RTV heap
		}
	}

	// Setup the secondary GPU - owns the display and swapchain
	{
		// Describe descriptor heaps - RTV and CBV/SRV/UAV
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = NumRenderTargets + 1;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
		cbvSrvUavHeapDesc.NumDescriptors = Secondary_Descriptor_Count;

		mDXDevices[Device_Secondary]->CreateDescriptorHeaps(rtvHeapDesc, cbvSrvUavHeapDesc);

		// Describe the SwapChain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = 0;

		CreateSwapChain(mDXDevices[Device_Secondary], swapChainDesc);
	}

	// Setup the cross adapter resources
	{
		mCrossAdapterResources = new DXCrossAdapterResources(mDXDevices[Device_Primary], mDXDevices[Device_Secondary]);
		mCrossAdapterResources->CreateResources();
	}
}

void DXMultiAdapterRenderer::CreateRootSignatures()
{
	mRootSignatures.resize(RootSignature_Count);

	// Root signature for scene on primary device
	{
		// Define root parameter 
		CD3DX12_DESCRIPTOR_RANGE ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // CBV for time
		CD3DX12_ROOT_PARAMETER rootParameters[1];
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc; // Root signature description
		rootSignatureDesc.Init(_countof(rootParameters),
			rootParameters,
			0,
			nullptr, // No static samplers are used
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// Serialize and create root signature
		ComPtr<ID3DBlob> signature, error;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ComPtr<ID3D12RootSignature> primaryRootSignature;
		ThrowIfFailed(mDXDevices[Device_Primary]->mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&primaryRootSignature)));
		mRootSignatures[Primary_RootSignature_Scene] = primaryRootSignature;
	}

	// Root signature for cross adapter resources on secondary device
	{
		// Define root parameters
		CD3DX12_DESCRIPTOR_RANGE ranges[2];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // CBV for transformation matrix
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // SRV for texture input
		CD3DX12_ROOT_PARAMETER crossAdapterRootParameters[2];
		crossAdapterRootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
		crossAdapterRootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);;

		// Setup static samplers - for accessing cross adapter textures
		CD3DX12_STATIC_SAMPLER_DESC staticTexturePointSampler(0);
		staticTexturePointSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		staticTexturePointSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		D3D12_STATIC_SAMPLER_DESC staticSamplers[] = { staticTexturePointSampler };

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc; // Root signature description
		rootSignatureDesc.Init(_countof(crossAdapterRootParameters),
			crossAdapterRootParameters,
			_countof(staticSamplers),
			staticSamplers,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// Serialize and create root signature
		ComPtr<ID3DBlob> signature, error;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ComPtr<ID3D12RootSignature> secondaryRootSignature;
		ThrowIfFailed(mDXDevices[Device_Secondary]->mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&secondaryRootSignature)));
		mRootSignatures[Secondary_RootSignature_CrossAdapter] = secondaryRootSignature;
	}

	// Root signature for scene on secondary device
	{
		// Define root parameters
		CD3DX12_DESCRIPTOR_RANGE ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // CBV for transformation matrix
		CD3DX12_ROOT_PARAMETER secondaryRootParameters[1];
		secondaryRootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc; // Root signature description
		rootSignatureDesc.Init(_countof(secondaryRootParameters),
			secondaryRootParameters,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// Serialize and create root signature
		ComPtr<ID3DBlob> signature, error;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ComPtr<ID3D12RootSignature> realsenseRootSignature;
		ThrowIfFailed(mDXDevices[Device_Secondary]->mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&realsenseRootSignature)));
		mRootSignatures[Secondary_RootSignature_Scene] = realsenseRootSignature;
	}
}

void DXMultiAdapterRenderer::LoadAssets()
{
	// Create root signatures for both devices 
	CreateRootSignatures();

	mPipelineStates.resize(PipelineState_Count); // Pre-allocate pipeline state vector
	mVertexBufferViews.resize(VertexBuffer_Count);

	// Vertex buffers and constant buffers for primary GPU
	{
		// Create vertex buffer for secondaryDevice GPU
		VertexUV quadVerticesTop[4];
		D3D12_VERTEX_BUFFER_VIEW* quadTopVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW();
		CreateVertexBuffer(mDXDevices[Device_Primary], quadTopVertexBufferView, &mQuadTopVertexBuffer, quadVerticesTop, sizeof(quadVerticesTop), sizeof(VertexUV));
		mVertexBufferViews[Primary_Quad_Top] = quadTopVertexBufferView;

		// Create constant buffer for time 
		UINT timeCbvHeapIndex = Primary_CBV_Time;
		CreateConstantBuffer(mDXDevices[Device_Primary], &mConstantBufferTimePrimary, &mConstantBufferTimePrimaryData, sizeof(ConstantBufferTime), timeCbvHeapIndex);
		mTimePrimaryCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDXDevices[Device_Primary]->mCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), timeCbvHeapIndex, mDXDevices[Device_Primary]->mCbvSrvUavDescriptorSize);
	}

	// Rendering PSO for primary GPU
	{
		// Describe PSO
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);


		// Define the vertex input layouts for the PSO
		D3D12_INPUT_ELEMENT_DESC inputLayoutDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// Load pre-compiled shaders 
		HRSRC resourcePixelShaderHandle = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(SCENEPIXELSHADERBLOB), RT_RCDATA);
		HGLOBAL memoryPixelShaderHandle = LoadResource(GetModuleHandle(NULL), resourcePixelShaderHandle);
		DWORD resourcePixelShaderSize = SizeofResource(GetModuleHandle(NULL), resourcePixelShaderHandle);
		LPVOID memoryPixelShaderAddress = LockResource(memoryPixelShaderHandle);

		HRSRC resourceVertexShaderHandle = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(SCENEVERTEXSHADERBLOB), RT_RCDATA);
		HGLOBAL memoryVertexShaderHandle = LoadResource(GetModuleHandle(NULL), resourceVertexShaderHandle);
		DWORD resourceVertexShaderSize = SizeofResource(GetModuleHandle(NULL), resourceVertexShaderHandle);
		LPVOID memoryVertexShaderAddress = LockResource(memoryVertexShaderHandle);

		psoDesc.VS = { reinterpret_cast<UINT8*>(memoryVertexShaderAddress), resourceVertexShaderSize };
		psoDesc.PS = { reinterpret_cast<UINT8*>(memoryPixelShaderAddress), resourcePixelShaderSize };
		std::wstring shaderPath; // This is an empty string

		// Create PSO
		ComPtr<ID3D12PipelineState> primaryPipelineState;
		CreatePipelineState(mDXDevices[Device_Primary], &primaryPipelineState, psoDesc, shaderPath, inputLayoutDesc, 2, mRootSignatures[Primary_RootSignature_Scene]);
		mPipelineStates[Primary_PipelineState_Scene] = primaryPipelineState;

		UnlockResource(memoryPixelShaderHandle);
		UnlockResource(memoryVertexShaderHandle);
	}

	// Vertex buffers and constant buffers for secondary GPU
	{
		// Vertices for bottom half of scene 
		VertexUV quadVerticesBottom[4];
		D3D12_VERTEX_BUFFER_VIEW* quadBottomVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW();
		CreateVertexBuffer(mDXDevices[Device_Secondary], quadBottomVertexBufferView, &mQuadBottomVertexBuffer, quadVerticesBottom, sizeof(quadVerticesBottom), sizeof(VertexUV));
		mVertexBufferViews[Secondary_Quad_Bottom] = quadBottomVertexBufferView;

		// Vertices for quad used to display cross-adapter texture
		VertexUV quadVerticesCombineTop[4];
		D3D12_VERTEX_BUFFER_VIEW* quadCombineTopVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW();
		CreateVertexBuffer(mDXDevices[Device_Secondary], quadCombineTopVertexBufferView, &mQuadCombineTopVertexBuffer, quadVerticesCombineTop, sizeof(quadVerticesCombineTop), sizeof(VertexUV));
		mVertexBufferViews[Secondary_Quad_Combine_Top] = quadCombineTopVertexBufferView;

		// Vertices for quad used to display scene from secondary GPU
		VertexUV quadVerticesCombineBottom[4];
		D3D12_VERTEX_BUFFER_VIEW* quadCombineBottomVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW();
		CreateVertexBuffer(mDXDevices[Device_Secondary], quadCombineBottomVertexBufferView, &mQuadCombineBottomVertexBuffer, quadVerticesCombineBottom, sizeof(quadVerticesCombineBottom), sizeof(VertexUV));
		mVertexBufferViews[Secondary_Quad_Combine_Bottom] = quadCombineBottomVertexBufferView;

		// Create constant buffer for time 
		UINT timeCbvHeapIndex = Secondary_CBV_Time;
		CreateConstantBuffer(mDXDevices[Device_Secondary], &mConstantBufferTimeSecondary, &mConstantBufferTimeSecondaryData, sizeof(ConstantBufferTime), timeCbvHeapIndex);
		mTimeSecondaryCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDXDevices[Device_Secondary]->mCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), timeCbvHeapIndex, mDXDevices[Device_Secondary]->mCbvSrvUavDescriptorSize);

		// Create constant buffer for transformation matrix for bottom combined quad
		UINT transformCbvHeapIndex = Secondary_CBV_Transform_Bottom;
		CreateConstantBuffer(mDXDevices[Device_Secondary], &mConstantBufferTransformBottom, &mConstantBufferTransformBottomData, sizeof(ConstantBufferTransform), transformCbvHeapIndex);
		mTransformBottomCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDXDevices[Device_Secondary]->mCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), transformCbvHeapIndex, mDXDevices[Device_Secondary]->mCbvSrvUavDescriptorSize);

		// Create constant buffer for transformation matrix for top combined quad
		UINT transformCACbvHeapIndex = Secondary_CBV_Transform_Top;
		CreateConstantBuffer(mDXDevices[Device_Secondary], &mConstantBufferTransformTop, &mConstantBufferTransformTopData, sizeof(ConstantBufferTransform), transformCACbvHeapIndex);
		mTransformTopCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDXDevices[Device_Secondary]->mCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), transformCACbvHeapIndex, mDXDevices[Device_Secondary]->mCbvSrvUavDescriptorSize);
	}

	// Rendering PSOs for secondary GPU
	{
		// Describe and create  PSO
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;

		// Set pipeline blend state
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.BlendState.RenderTarget[0].BlendEnable = false; // First render target has basic blending enabled
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

		// Define the vertex input layouts for the PSO
		D3D12_INPUT_ELEMENT_DESC inputLayoutDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// Load pre-compiled shaders 
		HRSRC resourceDisplayPixelShaderHandle = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(DISPLAYPIXELSHADERBLOB), RT_RCDATA);
		HGLOBAL memoryDisplayPixelShaderHandle = LoadResource(GetModuleHandle(NULL), resourceDisplayPixelShaderHandle);
		DWORD resourceDisplayPixelShaderSize = SizeofResource(GetModuleHandle(NULL), resourceDisplayPixelShaderHandle);
		LPVOID memoryDisplayPixelShaderAddress = LockResource(memoryDisplayPixelShaderHandle);

		HRSRC resourceDisplayVertexShaderHandle = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(DISPLAYVERTEXSHADERBLOB), RT_RCDATA);
		HGLOBAL memoryDisplayVertexShaderHandle = LoadResource(GetModuleHandle(NULL), resourceDisplayVertexShaderHandle);
		DWORD resourceDisplayVertexShaderSize = SizeofResource(GetModuleHandle(NULL), resourceDisplayVertexShaderHandle);
		LPVOID memoryDisplayVertexShaderAddress = LockResource(memoryDisplayVertexShaderHandle);

		psoDesc.VS = { reinterpret_cast<UINT8*>(memoryDisplayVertexShaderAddress), resourceDisplayVertexShaderSize };
		psoDesc.PS = { reinterpret_cast<UINT8*>(memoryDisplayPixelShaderAddress), resourceDisplayPixelShaderSize };

		std::wstring shaderPath; // This is an empty string

		ComPtr<ID3D12PipelineState> crossAdapterPipelineState;
		CreatePipelineState(mDXDevices[Device_Secondary], &crossAdapterPipelineState, psoDesc, shaderPath, inputLayoutDesc, 2, mRootSignatures[Secondary_RootSignature_CrossAdapter]);
		mPipelineStates[Secondary_PipelineState_CrossAdapter] = crossAdapterPipelineState;

		UnlockResource(memoryDisplayPixelShaderHandle);
		UnlockResource(memoryDisplayVertexShaderHandle);

		// Load pre-compiled shaders 
		HRSRC resourcePixelShaderHandle = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(SCENEPIXELSHADERBLOB), RT_RCDATA);
		HGLOBAL memoryPixelShaderHandle = LoadResource(GetModuleHandle(NULL), resourcePixelShaderHandle);
		DWORD resourcePixelShaderSize = SizeofResource(GetModuleHandle(NULL), resourcePixelShaderHandle);
		LPVOID memoryPixelShaderAddress = LockResource(memoryPixelShaderHandle);

		HRSRC resourceVertexShaderHandle = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(SCENEVERTEXSHADERBLOB), RT_RCDATA);
		HGLOBAL memoryVertexShaderHandle = LoadResource(GetModuleHandle(NULL), resourceVertexShaderHandle);
		DWORD resourceVertexShaderSize = SizeofResource(GetModuleHandle(NULL), resourceVertexShaderHandle);
		LPVOID memoryVertexShaderAddress = LockResource(memoryVertexShaderHandle);

		psoDesc.VS = { reinterpret_cast<UINT8*>(memoryVertexShaderAddress), resourceVertexShaderSize };
		psoDesc.PS = { reinterpret_cast<UINT8*>(memoryPixelShaderAddress), resourcePixelShaderSize };

		ComPtr<ID3D12PipelineState> realsensePipelineState;
		CreatePipelineState(mDXDevices[Device_Secondary], &realsensePipelineState, psoDesc, shaderPath, inputLayoutDesc, 2, mRootSignatures[Secondary_RootSignature_Scene]);
		mPipelineStates[Secondary_PipelineState_Scene] = realsensePipelineState;

		UnlockResource(memoryPixelShaderHandle);
		UnlockResource(memoryVertexShaderHandle);
	}

	CreateCommandLists();

	// Create texture for storing frame output from secondary GPU
	{
		D3D12_RESOURCE_DESC textureDesc = {}; // Describes texture resource
		textureDesc.Width = mWndProp.width;
		textureDesc.Height = mWndProp.height;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		// Create texture resource and SRV (handle in global variable)
		UINT textureSrvHeapIndex = Secondary_SRV_Texture;
		CreateTexture(mDXDevices[Device_Secondary], textureDesc, &mTexture, nullptr, textureSrvHeapIndex);
		mTextureSrvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDXDevices[Device_Secondary]->mCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), textureSrvHeapIndex, mDXDevices[Device_Secondary]->mCbvSrvUavDescriptorSize);

		// Set RTV handle for texture
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = textureDesc.Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		UINT textureRtvHeapIndex = 3; // 4th render target slot
		mTextureRtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDXDevices[Device_Secondary]->mRtvHeap->GetCPUDescriptorHandleForHeapStart(), textureRtvHeapIndex, mDXDevices[Device_Secondary]->mRtvDescriptorSize);
		mDXDevices[Device_Secondary]->mDevice->CreateRenderTargetView(mTexture.Get(), &rtvDesc, mTextureRtvHandle); // Create RTV in heap
	}

	// Setup cross adapter fences on each device
	mCrossAdapterResources->SetupFences();
}

void DXMultiAdapterRenderer::CreateCommandLists()
{
	mCommandLists.resize(CommandList_Count);

	// Command list for primary device rendering
	{
		ComPtr<ID3D12GraphicsCommandList> primaryCommandList;
		ThrowIfFailed(mDXDevices[Device_Primary]->mDevice->CreateCommandList(0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			mDXDevices[Device_Primary]->mCommandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(&primaryCommandList)));
		mCommandLists[Primary_CommandList_Scene] = primaryCommandList;

		ThrowIfFailed(mCommandLists[Primary_CommandList_Scene]->Close());
	}

	// Command list for copying between devices - in DXCrossAdapterResources object
	{
		mCrossAdapterResources->CreateCommandList();
	}

	// Command list for secondary device rendering
	{
		ComPtr<ID3D12GraphicsCommandList> secondaryCommandList;
		ThrowIfFailed(mDXDevices[Device_Secondary]->mDevice->CreateCommandList(0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			mDXDevices[Device_Secondary]->mCommandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(&secondaryCommandList)));
		mCommandLists[Secondary_CommandList_Scene] = secondaryCommandList;

		ThrowIfFailed(mCommandLists[Secondary_CommandList_Scene]->Close());
	}

	// Command list for displaying full scene (cross-adapter + secondary GPU resources)
	{
		ComPtr<ID3D12GraphicsCommandList> overlayCommandList;
		ThrowIfFailed(mDXDevices[Device_Secondary]->mDevice->CreateCommandList(0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			mDXDevices[Device_Secondary]->mCommandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(&overlayCommandList)));
		mCommandLists[Secondary_CommandList_Combine_Scene] = overlayCommandList;

		ThrowIfFailed(mCommandLists[Secondary_CommandList_Combine_Scene]->Close());
	}

}

void DXMultiAdapterRenderer::PopulateCommandLists()
{
	// Primary rendering command list
	{
		ComPtr<ID3D12Resource> curPrimaryRenderTarget = mDXDevices[Device_Primary]->mRenderTargets[mCurrentFrameIndex];

		// Reset allocator and command list for current render target
		ThrowIfFailed(mDXDevices[Device_Primary]->mCommandAllocator->Reset()); // Only do this when all command lists have finished executing
		ID3D12GraphicsCommandList* primaryCommandList = mCommandLists[Primary_CommandList_Scene].Get();
		ThrowIfFailed(primaryCommandList->Reset(mDXDevices[Device_Primary]->mCommandAllocator.Get(), nullptr));

		primaryCommandList->RSSetViewports(1, &mViewport);
		primaryCommandList->RSSetScissorRects(1, &mScissorRect);

		ID3D12DescriptorHeap* descHeaps[] = { mDXDevices[Device_Primary]->mCbvSrvUavHeap.Get() };
		primaryCommandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);

		// Allow for rending to current render target
		primaryCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(curPrimaryRenderTarget.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mDXDevices[Device_Primary]->mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mCurrentFrameIndex, mDXDevices[Device_Primary]->mRtvDescriptorSize);
		primaryCommandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
		primaryCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		primaryCommandList->IASetVertexBuffers(0, 1, mVertexBufferViews[Primary_Quad_Top]);

		// Render top portion of scene on primary GPU
		primaryCommandList->SetGraphicsRootSignature(mRootSignatures[Primary_RootSignature_Scene].Get());
		primaryCommandList->SetGraphicsRootDescriptorTable(0, mTimePrimaryCbvHandle);
		primaryCommandList->SetPipelineState(mPipelineStates[Primary_PipelineState_Scene].Get());
		primaryCommandList->DrawInstanced(4, 1, 0, 0);

		// Indicate that render target will be used for copy 
		primaryCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(curPrimaryRenderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ThrowIfFailed(mCommandLists[Primary_CommandList_Scene]->Close());
	}

	// Copy command list - in DXCrossAdapterResources object
	{
		mCrossAdapterResources->PopulateCommandList(mCurrentFrameIndex);
	}

	// Reset command allocator for secondary device - used by all following command lists	
	ThrowIfFailed(mDXDevices[Device_Secondary]->mCommandAllocator->Reset()); // Only do this when all command lists have finished executing

	// Secondary rendering command list
	{
		ID3D12GraphicsCommandList* secondaryCommandList = mCommandLists[Secondary_CommandList_Scene].Get();
		ThrowIfFailed(secondaryCommandList->Reset(mDXDevices[Device_Secondary]->mCommandAllocator.Get(), nullptr));

		secondaryCommandList->RSSetViewports(1, &mViewport);
		secondaryCommandList->RSSetScissorRects(1, &mScissorRect);

		ID3D12DescriptorHeap* descHeaps[] = { mDXDevices[Device_Secondary]->mCbvSrvUavHeap.Get() };
		secondaryCommandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);

		secondaryCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

		secondaryCommandList->OMSetRenderTargets(1, &mTextureRtvHandle, true, nullptr);
		secondaryCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		secondaryCommandList->IASetVertexBuffers(0, 1, mVertexBufferViews[Secondary_Quad_Bottom]); // Quad geometry

		// Render bottom portion of scene on secondary GPU
		secondaryCommandList->SetGraphicsRootSignature(mRootSignatures[Secondary_RootSignature_Scene].Get());
		secondaryCommandList->SetGraphicsRootDescriptorTable(0, mTimeSecondaryCbvHandle);
		secondaryCommandList->SetPipelineState(mPipelineStates[Secondary_PipelineState_Scene].Get());
		secondaryCommandList->DrawInstanced(4, 1, 0, 0); 

		secondaryCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		ThrowIfFailed(mCommandLists[Secondary_CommandList_Scene]->Close());
	}

	// Overlay Command List
	{
		ID3D12GraphicsCommandList* overlayCommandList = mCommandLists[Secondary_CommandList_Combine_Scene].Get();
		ComPtr<ID3D12Resource> curSecondaryRenderTarget = mDXDevices[Device_Secondary]->mRenderTargets[mCurrentFrameIndex]; // Get secondary render target for current frame
		ThrowIfFailed(overlayCommandList->Reset(mDXDevices[Device_Secondary]->mCommandAllocator.Get(), nullptr));

		overlayCommandList->RSSetViewports(1, &mViewport);
		overlayCommandList->RSSetScissorRects(1, &mScissorRect);

		ID3D12DescriptorHeap* descHeaps[] = { mDXDevices[Device_Secondary]->mCbvSrvUavHeap.Get() };
		overlayCommandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);

		overlayCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(curSecondaryRenderTarget.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mDXDevices[Device_Secondary]->mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mCurrentFrameIndex, mDXDevices[Device_Secondary]->mRtvDescriptorSize);
		overlayCommandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
		static const float clearColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
		overlayCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		overlayCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// Render top quad with cross-adapter texture
		overlayCommandList->IASetVertexBuffers(0, 1, mVertexBufferViews[Secondary_Quad_Combine_Top]); // Quad geometry
		CD3DX12_GPU_DESCRIPTOR_HANDLE crossAdapterSrvHandle(mDXDevices[Device_Secondary]->mCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), mCurrentFrameIndex, mDXDevices[Device_Secondary]->mCbvSrvUavDescriptorSize);
		overlayCommandList->SetGraphicsRootSignature(mRootSignatures[Secondary_RootSignature_CrossAdapter].Get());
		overlayCommandList->SetPipelineState(mPipelineStates[Secondary_PipelineState_CrossAdapter].Get());
		overlayCommandList->SetGraphicsRootDescriptorTable(0, mTransformTopCbvHandle);
		overlayCommandList->SetGraphicsRootDescriptorTable(1, crossAdapterSrvHandle);
		// Scale and translate quad to top of screen
		mConstantBufferTransformTopData.modelViewProjection = DirectX::XMMatrixIdentity();
		mConstantBufferTransformTopData.modelViewProjection *= DirectX::XMMatrixTranslation(0.0, ((1.0f / mSharePercentage) - 1.0f) + 0.015f, 0.0);
		mConstantBufferTransformTopData.modelViewProjection *= DirectX::XMMatrixScaling(1.0, mSharePercentage, 1.0);
		UpdateConstantBuffer(mConstantBufferTransformTop, &mConstantBufferTransformTopData, sizeof(mConstantBufferTransformTopData));
		overlayCommandList->DrawInstanced(4, 1, 0, 0);  

		// Render bottom quad with texture from secondary GPU
		overlayCommandList->IASetVertexBuffers(0, 1, mVertexBufferViews[Secondary_Quad_Combine_Bottom]); // Quad geometry
		overlayCommandList->SetGraphicsRootDescriptorTable(0, mTransformBottomCbvHandle);
		overlayCommandList->SetGraphicsRootDescriptorTable(1, mTextureSrvHandle);
		// Scale and translate quad to bottom of screen
		mConstantBufferTransformBottomData.modelViewProjection = DirectX::XMMatrixIdentity();
		mConstantBufferTransformBottomData.modelViewProjection *= DirectX::XMMatrixTranslation(0.0, -((1.0f / (1.0f - mSharePercentage)) - 1.0f) - 0.015f, 0.0);
		mConstantBufferTransformBottomData.modelViewProjection *= DirectX::XMMatrixScaling(1.0, (1.0f - mSharePercentage), 1.0);
		UpdateConstantBuffer(mConstantBufferTransformBottom, &mConstantBufferTransformBottomData, sizeof(mConstantBufferTransformBottomData));
		overlayCommandList->DrawInstanced(4, 1, 0, 0); 

		overlayCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(curSecondaryRenderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		ThrowIfFailed(mCommandLists[Secondary_CommandList_Combine_Scene]->Close());
	}

}

void DXMultiAdapterRenderer::ExecuteCommandLists()
{
	// Render scene on secondary device
	{
		ID3D12CommandList* primaryCommandLists[] = { mCommandLists[Secondary_CommandList_Scene].Get() };
		mDXDevices[Device_Secondary]->mCommandQueue->ExecuteCommandLists(_countof(primaryCommandLists), primaryCommandLists);
	}

	// Render scene on primary device and signal copy
	{
		ID3D12CommandList* primaryCommandLists[] = { mCommandLists[Primary_CommandList_Scene].Get() };
		mDXDevices[Device_Primary]->mCommandQueue->ExecuteCommandLists(_countof(primaryCommandLists), primaryCommandLists);;

		// Signal primary device command queue to indicate render is complete
		ThrowIfFailed(mDXDevices[Device_Primary]->mCommandQueue->Signal(mDXDevices[Device_Primary]->mFence.Get(), mCurrentFenceValue));
		mFenceValues[mCurrentFrameIndex] = mCurrentFenceValue;
		mCurrentFenceValue++;
	}

	// Copy resources on primary device into cross adapter resources
	{
		// Wait for primary device to finish rendering the frame
		ThrowIfFailed(mCrossAdapterResources->mCopyCommandQueue->Wait(mDXDevices[Device_Primary]->mFence.Get(), mFenceValues[mCurrentFrameIndex]));

		ID3D12CommandList* copyCommandLists[] = { mCrossAdapterResources->mCopyCommandList.Get() };
		mCrossAdapterResources->mCopyCommandQueue->ExecuteCommandLists(_countof(copyCommandLists), copyCommandLists);

		// Signal secondaryDevice adapter to indicate copy is complete
		ThrowIfFailed(mCrossAdapterResources->mCopyCommandQueue->Signal(mCrossAdapterResources->mFences[Device_Primary].Get(), mCrossAdapterResources->mCurrentFenceValue));
		mCrossAdapterResources->mFenceValues[mCurrentFrameIndex] = mCrossAdapterResources->mCurrentFenceValue;
		mCrossAdapterResources->mCurrentFenceValue++;

		// Secondary GPU wait for primaryDevice adapter to finish copying
		ThrowIfFailed(mDXDevices[Device_Secondary]->mCommandQueue->Wait(mCrossAdapterResources->mFences[Device_Secondary].Get(), mCrossAdapterResources->mFenceValues[mCurrentFrameIndex]));
	}

	// Render cross adapter resources and segmented texture overlay on secondary device
	{
		ID3D12CommandList* secondaryCommandLists[] = { mCommandLists[Secondary_CommandList_Combine_Scene].Get() };
		mDXDevices[Device_Secondary]->mCommandQueue->ExecuteCommandLists(_countof(secondaryCommandLists), secondaryCommandLists);;
	}
}

void DXMultiAdapterRenderer::MoveToNextFrame()
{
	// Schedule signal command in cross adapter resource queue
	mDXDevices[Device_Secondary]->mCommandQueue->Signal(mCrossAdapterResources->mFences[Device_Secondary].Get(), mCrossAdapterResources->mCurrentFenceValue);
	mCrossAdapterResources->mFenceValues[mCurrentFrameIndex] = mCrossAdapterResources->mCurrentFenceValue;
	mCrossAdapterResources->mCurrentFenceValue++;

	// Let the previous frame finish before continuing
	if (mCrossAdapterResources->mFences[Device_Primary]->GetCompletedValue() < mCrossAdapterResources->mFenceValues[mCurrentFrameIndex])
	{
		ThrowIfFailed(mCrossAdapterResources->mFences[Device_Primary]->SetEventOnCompletion(mCrossAdapterResources->mFenceValues[mCurrentFrameIndex], mCrossAdapterResources->mFenceEvents[Device_Primary]));
		WaitForSingleObjectEx(mCrossAdapterResources->mFenceEvents[Device_Primary], INFINITE, FALSE);
	}
}
