/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "PrecompiledHeaders.h"

// Custom DX12 render include
#include "DXDevice.h"
#include "DXUtil.h"
#include "DXMultiAdapterRenderer.h"

DXMultiAdapterRenderer* gRenderObj;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handle destroy/shutdown messages.
	switch (message)
	{
	case WM_KEYUP:
		if (gRenderObj == NULL)
			break;
		if (wParam == VK_OEM_PLUS || wParam == VK_DOWN)
			gRenderObj->IncrementSharePercentage();
		if (wParam == VK_OEM_MINUS || wParam == VK_UP)
			gRenderObj->DecrementSharePercentage();
		break;
	case WM_KEYDOWN:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Window information
	static TCHAR szWindowClass[] = _T("MA-DX12-Sample");
	static TCHAR szTitle[] = _T("MA-DX12-Sample");

	// Create window class
	WNDCLASSEX wincx;
	wincx.cbSize = sizeof(WNDCLASSEX);
	wincx.style = CS_HREDRAW | CS_VREDRAW;
	wincx.lpfnWndProc = WndProc;
	wincx.cbClsExtra = 0;
	wincx.cbWndExtra = 0;
	wincx.hInstance = hInstance;
	wincx.hIcon = NULL;
	wincx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wincx.lpszMenuName = NULL;
	wincx.lpszClassName = szWindowClass;
	wincx.hIconSm = NULL;

	// Register window
	if (!RegisterClassEx(&wincx))
	{
		MessageBox(NULL, _T("Unable to Register window!"), _T("MA-DX12 Sample"), NULL);
		return 1;
	}

	UINT windowHeight = 480;
	UINT windowWidth = 640;

	// Create the window
	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowWidth, windowHeight,
		NULL,
		NULL,
		hInstance,
		NULL
		);
	if (!hWnd)
	{
		MessageBox(NULL, _T("Unable to create window!"), _T("MA-DX12 Sample"), NULL);
		return 1;
	}

	ShowWindow(hWnd, nCmdShow);

#ifdef DEBUG
	// Enable the D3D12 debug layer - must be done before devices are made
	{
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			debugController->Release();
		}
	}
#endif

	// Create global DXGI Factory for enumerating adapters 
	MS::ComPtr<IDXGIFactory4> dxgiFactory;
	ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)));

	// Enumerate the primary adapter
	IDXGIAdapter* primaryAdapter;
	DXGI_ADAPTER_DESC primaryAdapterDesc;
	ThrowIfFailed(dxgiFactory->EnumAdapters(Device_Primary, &primaryAdapter)); // Get primaryDevice adapter ptr
	primaryAdapter->GetDesc(&primaryAdapterDesc);

	// Enumerate the secondary adapter - if only one GPU defaults to WARP
	IDXGIAdapter* secondaryAdapter;
	DXGI_ADAPTER_DESC secondaryAdapterDesc;
	HRESULT res = dxgiFactory->EnumAdapters(Device_Secondary, &secondaryAdapter); // Get secondaryDevice adapter ptr

	DXGI_ADAPTER_DESC usedAdapterDescPrimary, usedAdapterDescSecondary;
	// Create two DXDevices
	DXDevice* primaryDevice = nullptr;
	DXDevice* secondaryDevice = nullptr;
	std::vector<DXDevice*> devices;
	if (!SUCCEEDED(res))
	{
		primaryDevice = new DXDevice(primaryAdapter);
		devices.push_back(primaryDevice);
	}
	else
	{
		secondaryAdapter->GetDesc(&secondaryAdapterDesc);

		// For now check for NVIDIA card and ensure it is primary device
		if (secondaryAdapterDesc.VendorId == 4318)
		{
			primaryDevice = new DXDevice(secondaryAdapter);
			secondaryDevice = new DXDevice(primaryAdapter);
			usedAdapterDescPrimary = secondaryAdapterDesc;
			usedAdapterDescSecondary = primaryAdapterDesc;
			
		}
		else
		{
			primaryDevice = new DXDevice(primaryAdapter);
			secondaryDevice = new DXDevice(secondaryAdapter);
			usedAdapterDescPrimary = primaryAdapterDesc;
			usedAdapterDescSecondary = secondaryAdapterDesc;
		}

		devices.push_back(primaryDevice);
		devices.push_back(secondaryDevice);
	}

	// Create a DXRenderer object
	gRenderObj = new DXMultiAdapterRenderer(devices, dxgiFactory, windowWidth, windowHeight, hWnd);

	// Initialize the DX12 renderer
	gRenderObj->OnInit();

	// Variables for FPS
	DWORD dwFrames = 0;
	DWORD dwCurrentTime = 0;
	DWORD dwLastUpdateTime = 0;
	DWORD dwElapsedTime = 0;
	TCHAR szFPS[128];
	szFPS[0] = '\0';

	LARGE_INTEGER frequency, counter, elapsed, lastCounter;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);

	double framesPerSecond = 0.0, frameAccumulator = 0.0;
	int frameCounter = 0;
	// Main loop.
	MSG msg = { 0 };
	while (true)
	{
		lastCounter = counter;
		QueryPerformanceCounter(&counter);
		elapsed.QuadPart = counter.QuadPart - lastCounter.QuadPart;
		double curFrameTime = static_cast<double>(elapsed.QuadPart) / static_cast<double>(frequency.QuadPart);
		frameCounter++;
		frameAccumulator += curFrameTime;

		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}

		gRenderObj->OnUpdate();
		gRenderObj->OnRender();

		if (frameCounter == 15)
		{
			double frameTimeAvg = frameAccumulator / frameCounter;
			double framesPerSecondAvg = 1.0 / frameTimeAvg;
			int curSharePercentage = static_cast<int>(gRenderObj->GetSharePercentage() * 100.0);
			swprintf_s(szFPS, sizeof(szFPS) / sizeof(TCHAR), L"FPS = %.2f %s (%i%% Top) %s (%i%% Bottom)",
				static_cast<float>(framesPerSecondAvg),
				usedAdapterDescPrimary.Description,
				curSharePercentage,
				usedAdapterDescSecondary.Description,
				100 - curSharePercentage);
			frameAccumulator = frameTimeAvg;
			frameCounter = 1;
			// Write the FPS onto the window title.
			SetWindowText(hWnd, szFPS);
		}


	}

	gRenderObj->OnDestroy();
	delete gRenderObj, primaryDevice;
	if (!SUCCEEDED(res))
		delete secondaryDevice;
	return 0;
}

