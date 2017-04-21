/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// Useful enums for swapchain and multi adapter setup
enum Devices
{
	Device_Primary,
	Device_Secondary,
	Device_Count
};

enum RenderProperties
{
	NumRenderTargets = 3
};

namespace MS = Microsoft::WRL;

struct WindowProperties
{
	UINT width;
	UINT height;
	float aspectRatio;
	HWND hwnd;
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw;
	}
}
