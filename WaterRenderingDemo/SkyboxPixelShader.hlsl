//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

TextureCube skyMap : register(t[0]);

SamplerState samLinear : register(s[0]);

// A constant buffer.
cbuffer ModelViewProjectionConstantBuffer : register(b1)
{
	matrix model;
	matrix view;
	matrix projection;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_Position;
	float3 texCoord : TEXCOORD;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	//return float4(input.normal, 1.0f);
	return skyMap.Sample(samLinear, normalize(input.texCoord));
}
