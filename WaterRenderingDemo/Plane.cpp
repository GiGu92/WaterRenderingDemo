#include "pch.h"
#include "Plane.h"

#include "DDSTextureLoader.h"
#include "..\Helpers\DirectXHelper.h"

using namespace DirectX;

Plane::Plane(std::shared_ptr<DX::DeviceResources> deviceResources,
	int width, int height, float stride, 
	const wchar_t* diffuseTextureFile,
	const wchar_t* environmentTextureFile,
	const wchar_t* normalTextureFile)
	: SceneObject(deviceResources, nullptr, diffuseTextureFile, environmentTextureFile, normalTextureFile), 
	width(width), height(height), stride(stride) 
{ 
	GenerateMesh(deviceResources);
}

void Plane::GenerateMesh(std::shared_ptr<DX::DeviceResources> deviceResources)
{
	/*static const VertexPositionNormalTextureTangentBinormal planeVertices[] =
	{
		{ XMFLOAT3(-1.f, 0.f, -1.f), XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT2(0.f, 0.f) },
		{ XMFLOAT3( 1.f, 0.f, -1.f), XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT2(1.f, 0.f) },
		{ XMFLOAT3(-1.f, 0.f,  1.f), XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT2(0.f, 1.f) },
		{ XMFLOAT3( 1.f, 0.f,  1.f), XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT2(1.f, 1.f) }
	};*/
	UINT vbSize = (width + 1) * (height + 1);
	static VertexPositionNormalTextureTangentBinormal* planeVertices = new VertexPositionNormalTextureTangentBinormal[vbSize];
	XMFLOAT3 topLeftCorner(-width * stride / 2.f, 0, -height * stride / 2.f);
	for (int z = 0; z < height + 1; z++)
	{
		for (int x = 0; x < width + 1; x++)
		{
			planeVertices[(z*(width + 1)) + x] = VertexPositionNormalTextureTangentBinormal(
				XMFLOAT3(topLeftCorner.x + x * stride, 0, topLeftCorner.z + z * stride),
				XMFLOAT3(0.f, 1.f, 0.f),
				XMFLOAT2((float)x / (float)width, (float)z / (float)height),
				XMFLOAT3(1.f, 0.f, 0.f),
				XMFLOAT3(0.f, 0.f, -1.f));
		}
	}

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = planeVertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPositionNormalTextureTangentBinormal) * vbSize, D3D11_BIND_VERTEX_BUFFER);
	DX::ThrowIfFailed(
		deviceResources->GetD3DDevice()->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		&vertexBuffer
		)
		);

	/*static const unsigned short planeIndices[] =
	{
		0, 1, 2,
		1, 3, 2,
	};*/
	indexCount = width * height * 2 * 3;
	static unsigned int* planeIndices = new unsigned int[indexCount];
	for (int z = 0; z < height; z++)
	{
		for (int x = 0; x < width; x++)
		{
			planeIndices[(z*width + x) * 6] = z * (width + 1) + x;
			planeIndices[(z*width + x) * 6 + 1] = z * (width + 1) + x + 1;
			planeIndices[(z*width + x) * 6 + 2] = (z + 1) * (width + 1) + x;

			planeIndices[(z*width + x) * 6 + 3] = (z + 1) * (width + 1) + x + 1;
			planeIndices[(z*width + x) * 6 + 4] = (z + 1) * (width + 1) + x;
			planeIndices[(z*width + x) * 6 + 5] = z * (width + 1) + x + 1;
		}
	}

	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = planeIndices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * indexCount, D3D11_BIND_INDEX_BUFFER);
	DX::ThrowIfFailed(
		deviceResources->GetD3DDevice()->CreateBuffer(
		&indexBufferDesc,
		&indexBufferData,
		&indexBuffer
		)
		);
}

void Plane::Draw(std::shared_ptr<DX::DeviceResources> deviceResources)
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

	UINT stride = sizeof(VertexPositionNormalTextureTangentBinormal);
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
		DXGI_FORMAT_R32_UINT,
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
		0,
		1,
		//psConstantBuffer.GetAddressOf()
		vsConstantBuffer.GetAddressOf()
		);

	context->PSSetShaderResources(0, 1, diffuseTexture.GetAddressOf());
	context->PSSetShaderResources(1, 1, environmentTexture.GetAddressOf());
	context->PSSetShaderResources(2, 1, normalTexture.GetAddressOf());
	context->PSSetSamplers(0, 1, linearSampler.GetAddressOf());

	// Draw the objects.
	context->DrawIndexed(
		indexCount,
		0,
		0
		);
}

void Plane::LoadVS(
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
		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	DX::ThrowIfFailed(
		deviceResources->GetD3DDevice()->CreateInputLayout(
		vertexDesc,
		ARRAYSIZE(vertexDesc),
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

Plane::~Plane()
{
}
