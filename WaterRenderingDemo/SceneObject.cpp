#include "pch.h"
#include "SceneObject.h"

#include "..\Helpers\DirectXHelper.h"

using namespace WaterRenderingDemo;

using namespace DirectX;
using namespace Windows::Foundation;

SceneObject::SceneObject() { }

SceneObject::SceneObject(std::shared_ptr<DX::DeviceResources> deviceResources, const wchar_t* modelFile)
{
	auto device = deviceResources->GetD3DDevice();
	EffectFactory fx(device);
	this->model = Model::CreateFromCMO(device, modelFile, fx, false);

	//XMStoreFloat4x4(&this->world, XMMatrixIdentity());
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
	//this->model->Draw(context, states, local, XMLoadFloat4x4(&constantBufferData.view), XMLoadFloat4x4(&constantBufferData.projection), false);

	XMStoreFloat4x4(&this->vsConstantBufferData.model, local);

	for each (auto& mesh in model->meshes)
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

			//device->CreateInputLayout(
			//	VertexPositionNormalTexture::InputElements,
			//	VertexPositionNormalTexture::InputElementCount,
			//	vsByteCode, vsByteCodeLength,
			//	&inputLayout);

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

			// Draw the objects.
			context->DrawIndexed(
				part->indexCount,
				part->startIndex,
				0
				);
		}
	}
	
}

/*void SceneObject::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext2> context)
{
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		vertexBuffer.GetAddressOf(),
		&stride,
		&offset
		);

	context->IASetIndexBuffer(
		indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
		);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(inputLayout.Get());

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
		constantBuffer.GetAddressOf()
		);

	// Attach our pixel shader.
	context->PSSetShader(
		pixelShader.Get(),
		nullptr,
		0
		);

	// Draw the objects.
	context->DrawIndexed(
		indexCount,
		0,
		0
		);
}*/

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
		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,    0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	DX::ThrowIfFailed(
		deviceResources->GetD3DDevice()->CreateInputLayout(
		vertexDesc,
		ARRAYSIZE(vertexDesc),
		//VertexPositionNormalTexture::InputElements,
		//VertexPositionNormalTexture::InputElementCount,
		&vsFileData[0],
		vsFileData.size(),
		&inputLayout
		)
		);
}

void SceneObject::LoadPS(
	std::shared_ptr<DX::DeviceResources> deviceResources,
	const std::vector<byte>& psFileData)
{
	DX::ThrowIfFailed(
		deviceResources->GetD3DDevice()->CreatePixelShader(
		&psFileData[0],
		psFileData.size(),
		nullptr,
		&pixelShader
		)
		);

	CD3D11_BUFFER_DESC vsConstantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		deviceResources->GetD3DDevice()->CreateBuffer(
		&vsConstantBufferDesc,
		nullptr,
		&vsConstantBuffer
		)
		);

	CD3D11_BUFFER_DESC psConstantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		deviceResources->GetD3DDevice()->CreateBuffer(
		&psConstantBufferDesc,
		nullptr,
		&psConstantBuffer
		)
		);

	auto device = deviceResources->GetD3DDevice();
	auto context = deviceResources->GetD3DDeviceContext();

	this->states = std::shared_ptr<CommonStates>(new CommonStates(device));
	context->RSSetState(states->CullNone());
	context->OMSetBlendState(states->AlphaBlend(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(states->DepthRead(), 0);
	
}

void SceneObject::LoadMesh(
	std::shared_ptr<DX::DeviceResources> deviceResources,
	const wchar_t* modelFile)
{
	auto device = deviceResources->GetD3DDevice();
	EffectFactory fx(device);
	this->model = Model::CreateFromCMO(device, modelFile, fx, false);
}