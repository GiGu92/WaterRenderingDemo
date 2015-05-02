//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

Texture2D diffuseMap : register(t[0]);
TextureCube environmentMap : register(t[1]);
Texture2D normalMap : register(t[2]);

SamplerState samLinear : register(s[0]);

// A constant buffer.
cbuffer MyConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 cameraPos;
	float4 lightPos;
	float4 lightColor;
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

float4 ComputeIllumination(float2 texCoord, float3 vLightTS, float3 vViewTS, float3 vNormalWS)
{
	// Sample the normal from the normal map for the given texture sample:
	//float3 vNormalTS = normalize(normalMap.Sample(samLinear, texCoord * 10) * 2.0 - 1.0);
	//float3 vNormalTS = vNormalWS;
	float3 vNormalTS = float3(0,1,0);

	// Setting base color
	float4 cBaseColor = float4(1, 1, 1, 1);

	// Compute ambient component
	float ambientPower = 0.1f;
	float4 ambientColor = float4(1, 1, 1, 1);
	float4 cAmbient = ambientColor * ambientPower;

	// Compute diffuse color component:
	float4 cDiffuse = saturate(dot(vNormalTS, vLightTS));

	// Compute the specular component if desired:
	float3 H = normalize(vLightTS + vViewTS);
	float NdotH = max(0, dot(vNormalTS, H));
	float shininess = 40;
	float specularPower = 100000;
	float4 specularColor = float4(1, 1, 1, 1);
	float4 cSpecular = pow(saturate(NdotH), shininess) * specularColor * specularPower;

	// Composite the final color:
	float4 cFinalColor = (cAmbient + cDiffuse) * cBaseColor/* + cSpecular*/;

	return cFinalColor;
}

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	//return float4(input.normal, 1.0f);
	//float4 color = environmentMap.Sample(samLinear, input.reflectionWS);
	float4 lightingColor = ComputeIllumination(input.texCoord, normalize(input.lightTS), normalize(input.viewTS), input.normalWS);

	float3 viewDir = normalize(input.posWS - cameraPos);
	float3 normalTS = normalize(normalMap.Sample(samLinear, input.texCoord * 10) * 2.0 - 1.0);
	float3 reflectionWS = reflect(viewDir, normalTS);
	//float3 reflectionWS = reflect(viewDir, input.normalWS);
	float4 reflectionColor = environmentMap.Sample(samLinear, reflectionWS);

	float4 color = /*lightingColor * */reflectionColor;
	//float4 color = lightingColor/* * reflectionColor*/;
	color.a = 0.5f;

	return color;
}
