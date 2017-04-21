/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

static const float g_voxelSize = 0.2;
static const float g_voxelGridSize = 5.0 * g_voxelSize;
static const float g_voxelSizeHalf = g_voxelSize / 2.0;

static const int g_steps = 200;
static const float g_rayStep = .01;
static const int g_light_steps = 200;
static const int g_bounce_steps = 100;
static const float g_ambientLight = .10;
static const float g_bounceContribution = .10;

int VoxelLookup(int voxel)
{
	if (0 == voxel) return 1;
	if (1 == voxel) return 1;
	if (2 == voxel) return 0;
	if (3 == voxel) return 1;
	if (4 == voxel) return 1;

	if (5 == voxel) return 0;
	if (6 == voxel) return 1;
	if (7 == voxel) return 0;
	if (8 == voxel) return 1;
	if (9 == voxel) return 0;

	if (10 == voxel) return 0;
	if (11 == voxel) return 0;
	if (12 == voxel) return 1;
	if (13 == voxel) return 0;
	if (14 == voxel) return 0;

	if (15 == voxel) return 0;
	if (16 == voxel) return 1;
	if (17 == voxel) return 0;
	if (18 == voxel) return 1;
	if (19 == voxel) return 0;

	if (20 == voxel) return 1;
	if (21 == voxel) return 1;
	if (22 == voxel) return 0;
	if (23 == voxel) return 1;
	if (24 == voxel) return 1;

	return 0;
}

float4 VoxelLookup_Color(int voxel)
{
	if (0 == voxel) return float4(1, 1, 1, 1);
	if (1 == voxel) return float4(1, 0, 0, 1);
	if (2 == voxel) return float4(0, 0, 0, 0);
	if (3 == voxel) return float4(1, 0, 0, 1);
	if (4 == voxel) return float4(1, 1, 1, 1);

	if (5 == voxel) return float4(0, 0, 0, 0);
	if (6 == voxel) return float4(1, 1, 1, 1);
	if (7 == voxel) return float4(0, 0, 0, 0);
	if (8 == voxel) return float4(1, 1, 1, 1);
	if (9 == voxel) return float4(0, 0, 0, 0);

	if (10 == voxel) return float4(0, 0, 0, 0);
	if (11 == voxel) return float4(0, 0, 0, 0);
	if (12 == voxel) return float4(1, 1, 1, 1);
	if (13 == voxel) return float4(0, 0, 0, 0);
	if (14 == voxel) return float4(0, 0, 0, 0);

	if (15 == voxel) return float4(0, 0, 0, 0);
	if (16 == voxel) return float4(1, 1, 1, 1);
	if (17 == voxel) return float4(0, 0, 0, 0);
	if (18 == voxel) return float4(1, 1, 1, 1);
	if (19 == voxel) return float4(0, 0, 0, 0);

	if (20 == voxel) return float4(1, 1, 1, 1);
	if (21 == voxel) return float4(1, 0, 0, 1);
	if (22 == voxel) return float4(0, 0, 0, 0);
	if (23 == voxel) return float4(1, 0, 0, 1);
	if (24 == voxel) return float4(1, 1, 1, 1);

	return float4(0, 0, 0, 0);
}

float mod(float x, float y)
{
	return x - y * floor(x / y);
}

bool IsInVoxel(float3 rayDirection, float3 pos, int voxel, out float3 normal, out float4 fragColor)
{
	normal = float3(0.0, 0.0, 0.0);
	fragColor = float4(0.0, 0.0, 0.0, 0.0);
	float3 center;

	float fv = float(voxel);

	float x = mod(fv, 5.0);
	float y = mod(fv, 25.0);

	center.x = x / 4.0 * 4.0 - 2.0;
	center.y = floor(y / 5.0) / 4.0 * 4.0 - 2.0;
	center.z = floor(fv / 25.0);

	center = center * g_voxelSize;

	float3 delta = pos - center;
	float3 absDelta = abs(delta);

	if (absDelta.x > g_voxelSizeHalf)
		return false;

	if (absDelta.y > g_voxelSizeHalf)
		return false;

	if (absDelta.z > g_voxelSizeHalf)
		return false;

	if (VoxelLookup(voxel) > 0)
	{
		fragColor = VoxelLookup_Color(voxel);

		//back our position up one step to the previous position
		float3 posCubeSpace = delta - rayDirection * g_rayStep;

		normal = float3(0, 0, 0);

		//for any plane that our previous position was above, include its normal
		//into our normal calculation
		if (dot(posCubeSpace, float3(1, 0, 0)) + -g_voxelSizeHalf > 0.0)
			normal += float3(1, 0, 0);
		else if (dot(posCubeSpace, float3(0, 1, 0)) + -g_voxelSizeHalf > 0.0)
			normal += float3(0, 1, 0);
		else if (dot(posCubeSpace, float3(0, 0, 1)) + -g_voxelSizeHalf > 0.0)
			normal += float3(0, 0, 1);
		else if (dot(posCubeSpace, float3(-1, 0, 0)) + -g_voxelSizeHalf > 0.0)
			normal += float3(-1, 0, 0);
		else if (dot(posCubeSpace, float3(0, -1, 0)) + -g_voxelSizeHalf > 0.0)
			normal += float3(0, -1, 0);
		else if (dot(posCubeSpace, float3(0, 0, -1)) + -g_voxelSizeHalf > 0.0)
			normal += float3(0, 0, -1);

		normal = normalize(normal);

		return true;
	}

	return false;
}

bool IsInGrid(float3 pos)
{
	if (abs(pos.x) > g_voxelGridSize / 2.0)
		return false;

	if (abs(pos.y) > g_voxelGridSize / 2.0)
		return false;

	if (abs(pos.z) > g_voxelGridSize / 2.0)
		return false;

	return true;
}

bool TestVoxels(float3 rayDirection, float3 pos, out float3 normal, out float4 fragColor)
{
	normal = float3(0.0, 0.0, 0.0);
	fragColor = float4(0.0, 0.0, 0.0, 0.0);
	if (pos.y < -.5)
	{
		fragColor = float4(.7, 1, 1, 1);
		normal = float3(0, 1.0, 0);
		return true;
	}

	if (false == IsInGrid(pos)) return false;

	if (IsInVoxel(rayDirection, pos, 0, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 1, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 2, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 3, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 4, normal, fragColor)) return true;

	if (IsInVoxel(rayDirection, pos, 5, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 6, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 7, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 8, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 9, normal, fragColor)) return true;

	if (IsInVoxel(rayDirection, pos, 10, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 11, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 12, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 13, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 14, normal, fragColor)) return true;

	if (IsInVoxel(rayDirection, pos, 15, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 16, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 17, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 18, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 19, normal, fragColor)) return true;

	if (IsInVoxel(rayDirection, pos, 20, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 21, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 22, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 23, normal, fragColor)) return true;
	if (IsInVoxel(rayDirection, pos, 24, normal, fragColor)) return true;

	return false;
}

// HLSL added

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer ConstantBuffer : register(b0)
{
	float time;
};

float4 PShader(PixelShaderInput input) : SV_TARGET
{
	float4 fragColor;
	float2 resolution = float2(640.0, 480.0);
	float2 fragCoord = float2(input.position[0], resolution.y - input.position[1]);
	float camRot = 2.7;
	float lightRot = 0.7070;

	//lightRot = time;
	//camRot = time;
	//camRot = iMouse.x * .01;

	float3 pixel;
	pixel.xy = input.uv.xy * 2.0 - 1.0;
	pixel.z = 0.0;

	float3 eye;
	eye.x = 0.0;
	eye.y = 0.0;
	eye.z = -1.0;

	float3 light = float3(0.0, 1.0, 1.0);
	float3 dir = normalize(pixel - eye);
	float3 pos = float3(0, 0, -3.0);
	pos.z = -1.0;

	float3 dr = dir;
	dr.x = cos(camRot) * dir.x + -sin(camRot) * dir.z;
	dr.z = sin(camRot) * dir.x + cos(camRot) * dir.z;

	float3 pr = pos;
	pr.x = cos(camRot) * pos.x + -sin(camRot) * pos.z;
	pr.z = sin(camRot) * pos.x + cos(camRot) * pos.z;

	float3 lr = light;
	lr.x = cos(lightRot) * light.x + -sin(lightRot) * light.z;
	lr.z = sin(lightRot) * light.x + cos(lightRot) * light.z;

	float3 light_vec = normalize(lr);

	float4 voxelColor = float4(0.0, 0.0, 0.0, 0.0);
	float3 normal = float3(0.0, 0.0, 0.0);

	for (int i = 0; i < g_steps; i++)
	{
		if (true == TestVoxels(dr, pr, normal, voxelColor))
		{
			float4 bounceColor = float4(0.0, 0.0, 0.0, 0.0);

			//back the ray back out of our collision
			pr -= dr * g_rayStep;

			float3 impactPoint = pr;

			//and now go towards the light
			float3 to_light = normalize(lr - pr);

			float4 diffuse = voxelColor * dot(light_vec, normal);
			float4 ambient = voxelColor * g_ambientLight;
			float4 bounce = float4(0, 0, 0, 0);

			float3 ignore_normal;
			for (int c = 0; c < g_light_steps; c++)
			{
				pr += to_light * g_rayStep;

				if (true == TestVoxels(to_light, pr, ignore_normal, bounceColor))
				{
					diffuse = float4(0, 0, 0, 0);
					break;
				}
			}

			pr = impactPoint;

			if (pr.y < -.48)
			{
				float3 reflection = dr - 2.0 * dot(normal, dr) * normal;

				for (int c = 0; c < g_bounce_steps; c++)
				{
					pr += reflection * g_rayStep;

					if (true == TestVoxels(reflection, pr, ignore_normal, bounceColor))
					{
						bounce = bounceColor * dot(light_vec, ignore_normal) * g_bounceContribution;
						break;
					}
				}
			}
			voxelColor = bounce + diffuse + ambient;

			break;
		}

		pr += dr * g_rayStep;
	}

	fragColor = voxelColor;
	return fragColor;
}
