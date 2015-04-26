//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : SV_Position;
	float3 normal : NORMAL;
	//float4 tangent : TANGENT0;
	//float color : COLOR0;
	float2 texCoord : TEXCOORD;
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

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

		// Transform the vertex position into projected space.
		float4x4 worldviewproj = mul(mul(projection, view), model);
		output.pos = mul(worldviewproj, pos);

	// Pass the color through without modification.
	output.texCoord = input.texCoord;
	output.normal = input.normal;

	return output;
}
