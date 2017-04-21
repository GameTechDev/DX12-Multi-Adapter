/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer ConstantBufferTransform : register(b0)
{
	float4x4 modelViewProjection;
};

Texture2D InputTexture : register(t0);
SamplerState InputPointSampler : register(s0);

float4 PShader(PixelShaderInput input) : SV_TARGET
{
	float2 uv = input.uv;
	// Flip UV coordinates of texture
	uv.y = 1 - uv.y;
	float3 textureColor = InputTexture.Sample(InputPointSampler, uv).xyz;
	return float4(textureColor, 1.0);
}
