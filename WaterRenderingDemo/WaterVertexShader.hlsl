//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer MyConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 cameraPos;
	float4 lightPos;
	float4 lightColor;
	float4 totalTime;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 posOS : SV_Position;
	float3 normalOS : NORMAL;
	float2 texCoord : TEXCOORD;
	float3 tangentOS : TANGENT;
	float3 binormalOS : BINORMAL;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 posPS : SV_Position;
	float3 posWS : POSITION;
	float3 normalWS : NORMAL;
	float2 texCoord : TEXCOORD0;
	float3 viewTS : VIEWVECTORS;
	float3 lightTS : LIGHTVECTORS;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 posOS = float4(input.posOS, 1.0f);

	// Calculate waves
	float4 SpaceFreq = float4(3.5f, 0.0f, 3.5f, 0.0f);
	float4 TimeFreq = float4(1.4f, 0.0f, 1.4f, 0.0f);
	float4 Amplitudes = float4(0.08f, 0.0f, 0.16f, 0.0f);
	float4 WaveDirX = float4(-0.5f, 0.0f, 0.1f, 0.0f);
	float4 WaveDirZ = float4(0.7f, 0.0f, 0.5f, 0.0f);
	float Time = totalTime.x;
	float4 Phase = (WaveDirX * posOS.x + WaveDirZ * posOS.z) * SpaceFreq + Time * TimeFreq;
	float4 Cos, Sin;
	sincos(Phase, Sin, Cos);
	float WaveHeight = dot(Sin, Amplitudes);

	posOS.y += WaveHeight;

	float4 CosWaveHeight = WaveHeight * SpaceFreq * Cos;
	float3 tangentOS = float3(0, dot(CosWaveHeight, WaveDirZ), 1);
	float3 binormalOS = float3(1, dot(CosWaveHeight, WaveDirX), 0);
	float3 normalOS = cross(tangentOS, binormalOS);

	// Transform the vertex position into projected space.
	float4x4 worldviewproj = mul(mul(projection, view), model);
	output.posPS = mul(worldviewproj, posOS);

	// Pass normal and texcoord through
	float3 normalWS = normalize(mul(model, normalOS));
	output.normalWS = normalWS;
	output.texCoord = input.texCoord;

	// Calculate world position
	float4 posWS = mul(model, posOS);
	posWS.y += WaveHeight;

	output.posWS = posWS.xyz;

	// Calculate tangent basis
	float3 tangentWS = normalize(mul(model, tangentOS));
	float3 binormalWS = normalize(mul(model, binormalOS));
	float3x3 WorldToTangent = transpose(float3x3(tangentWS, binormalWS, normalWS));
	
	// Calculate tangent space view and light vectors
	float3 lightWS = normalize(lightPos.xyz - posWS.xyz);
	output.lightTS = mul(WorldToTangent, lightWS).yxz;
	float3 viewWS = normalize(cameraPos.xyz - posWS.xyz);
	output.viewTS = mul(WorldToTangent, viewWS);

	return output;
}
