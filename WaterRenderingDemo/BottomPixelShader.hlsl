//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

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
	float3 normal : NORMAL;
	//float4 tangent : TANGENT0;
	//float color : COLOR0;
	float2 texCoord : TEXCOORD;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	//return float4(input.texCoord, 1.0f, 1.0f);
	return float4(input.texCoord, 0.0f, 1.0f);
}
