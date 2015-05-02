#include "pch.h"
#include "SceneObject.h"

#include "..\Helpers\DirectXHelper.h"

#include "DDSTextureLoader.h"

using namespace WaterRenderingDemo;

using namespace DirectX;
using namespace Windows::Foundation;

SceneObject::SceneObject() { }

SceneObject::SceneObject(std::shared_ptr<DX::DeviceResources> deviceResources, 
	const wchar_t* modelFile, 
	const wchar_t* diffuseTextureFile,
	const wchar_t* environmentTextureFile,
	const wchar_t* normalTextureFile)
{
	auto device = deviceResources->GetD3DDevice();

	// Load mesh
	if (modelFile != nullptr)
	{
		this->LoadMesh(deviceResources, modelFile);
	}

	// Load textures
	if (diffuseTextureFile != nullptr)
	{
		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(device, diffuseTextureFile, nullptr, diffuseTexture.ReleaseAndGetAddressOf())
			);
	}
	if (environmentTextureFile != nullptr)
	{
		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(device, environmentTextureFile, nullptr, environmentTexture.ReleaseAndGetAddressOf())
			);
	}
	if (normalTextureFile != nullptr)
	{
		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(device, normalTextureFile, nullptr, normalTexture.ReleaseAndGetAddressOf())
			);
	}

	// Create samplers
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampDesc, &linearSampler);

}

SceneObject::~SceneObject()
{
	vertexShader.Reset();
	pixelShader.Reset();
	vsConstantBuffer.Reset();
	psConstantBuffer.Reset();
}

void SceneObject::Draw(std::shared_ptr<DX::DeviceResources> deviceResources)
{
	auto device = deviceResources->GetD3DDevice();
	auto context = deviceResources->GetD3DDeviceContext();
	CommonStates states(device);

	context->UpdateSubresource(
		this->vsConstantBuffer.Get(),
		0,
		NULL,
		&this->vsConstantBufferData,
		0,
		0);

	context->UpdateSubresource(
		this->psConstantBuffer.Get(),
		0,
		NULL,
		&this->psConstantBufferData,
		0,
		0);
		
	//model->Draw(context, states, local, view, proj);
	XMVECTOR qid = XMQuaternionIdentity();
	const XMVECTORF32 scale = { 1.f, 1.f, 1.f };
	const XMVECTORF32 translate = { 0.f, 0.f, 0.f };
	//XMVECTOR rotate = XMQuaternionRotationRollPitchYaw(0, XM_PI / 2.f, -XM_PI / 2.f);
	XMVECTOR rotate = XMQuaternionRotationRollPitchYaw(0, 0, 0);
	XMMATRIX worldMatrix = XMLoadFloat4x4(&this->vsConstantBufferData.model);
	XMMATRIX local = XMMatrixMultiply(worldMatrix, XMMatrixTransformation(g_XMZero, qid, scale, g_XMZero, rotate, translate));
	//this->model->Draw(context, states, local, XMLoadFloat4x4(&vsConstantBufferData.view), XMLoadFloat4x4(&vsConstantBufferData.projection), false);

	XMStoreFloat4x4(&this->vsConstantBufferData.model, local);

	for each(auto& mesh in model->meshes)
	{
		for each (auto& part in mesh->meshParts)
		{
			context->IASetVertexBuffers(
				0,
				1,
				part->vertexBuffer.GetAddressOf(),
				&part->vertexStride,
				&part->vertexOffset
				);

			context->IASetIndexBuffer(
				part->indexBuffer.Get(),
				part->indexFormat,
				0
				);

			context->IASetPrimitiveTopology(part->primitiveType);

			context->IASetInputLayout(inputLayout.Get());
			//context->IASetInputLayout(part->inputLayout.Get());

			// Attach our vertex shader.
			context->VSSetShader(
				vertexShader.Get(),
				nullptr,
				0
				);

			// Send the constant buffer to the graphics device.
			context->VSSetConstantBuffers(
				0,
				1,
				vsConstantBuffer.GetAddressOf()
				);

			// Attach our pixel shader.
			context->PSSetShader(
				pixelShader.Get(),
				nullptr,
				0
				);

			// Send the constant buffer to the graphics device.
			context->PSSetConstantBuffers(
				1,
				1,
				psConstantBuffer.GetAddressOf()
				);

			context->PSSetShaderResources(0, 1, diffuseTexture.GetAddressOf());
			context->PSSetSamplers(0, 1, linearSampler.GetAddressOf());

			// Draw the objects.
			context->DrawIndexed(
				part->indexCount,
				part->startIndex,
				0
				);
		}
	}
	
}

void SceneObject::LoadVS(
	std::shared_ptr<DX::DeviceResources> deviceResources,
	const std::vector<byte>& vsFileData)
{
	DX::ThrowIfFailed(
		deviceResources->GetD3DDevice()->CreateVertexShader(
		&vsFileData[0],
		vsFileData.size(),
		nullptr,
		&vertexShader
		)
		);

	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",       0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	DX::ThrowIfFailed(
		deviceResources->GetD3DDevice()->CreateInputLayout(
		vertexDesc,
		ARRAYSIZE(vertexDesc),
		//VertexPositionNormalTexture::InputElements,
		//VertexPositionNormalTexture::InputElementCount,
		//VertexPositionNormalTangentColorTexture::InputElements,
		//VertexPositionNormalTangentColorTexture::InputElementCount,
		&vsFileData[0],
		vsFileData.size(),
		&inputLayout
		)
		);

	CD3D11_BUFFER_DESC vsConstantBufferDesc(sizeof(MyConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		deviceResources->GetD3DDevice()->CreateBuffer(
		&vsConstantBufferDesc,
		nullptr,
		&vsConstantBuffer
		)
		);
}

void SceneObject::LoadPS(
	std::shared_ptr<DX::DeviceResources> deviceResources,
	const std::vector<byte>& psFileData)
{
	auto device = deviceResources->GetD3DDevice();
	auto context = deviceResources->GetD3DDeviceContext();

	DX::ThrowIfFailed(
		device->CreatePixelShader(
		&psFileData[0],
		psFileData.size(),
		nullptr,
		&pixelShader
		)
		);

	CD3D11_BUFFER_DESC psConstantBufferDesc(sizeof(MyConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		device->CreateBuffer(
		&psConstantBufferDesc,
		nullptr,
		&psConstantBuffer
		)
		);	
}

void SceneObject::LoadMesh(
	std::shared_ptr<DX::DeviceResources> deviceResources,
	const wchar_t* modelFile)
{
	auto device = deviceResources->GetD3DDevice();
	EffectFactory fx(device);
	this->model = Model::CreateFromCMO(device, modelFile, fx, true);
}