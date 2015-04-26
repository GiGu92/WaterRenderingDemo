//--------------------------------------------------------------------------------------
// File: DirectXTK3DSceneRenderer.cpp
//
// This is a simple Windows Store app for Windows 8.1 showing use of DirectXTK
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "DirectXTK3DSceneRenderer.h"

#include "DDSTextureLoader.h"

#include "Helpers\DirectXHelper.h"	// For ThrowIfFailed and ReadDataAsync

using namespace WaterRenderingDemo;

using namespace DirectX;
using namespace Windows::Foundation;

DirectXTK3DSceneRenderer::DirectXTK3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
m_deviceResources(deviceResources)
{
	m_water = std::shared_ptr<SceneObject>(new SceneObject(deviceResources, L"plane.cmo", false));
	m_bottom = std::shared_ptr<SceneObject>(new SceneObject(deviceResources, L"plane.cmo"));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void DirectXTK3DSceneRenderer::CreateWindowSizeDependentResources()
{
	camera = std::unique_ptr<Camera>(new Camera(
		XMFLOAT4(0.0f, 7.f, 15.f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),
		m_deviceResources));

	m_batchEffect->SetProjection(camera->getProjection());

	XMStoreFloat4x4(&m_water->vsConstantBufferData.projection, camera->getProjection());
	XMStoreFloat4x4(&m_bottom->vsConstantBufferData.projection, camera->getProjection());

}

void DirectXTK3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	camera->Update(timer);

	m_batchEffect->SetView(camera->getView());
	m_batchEffect->SetWorld(XMMatrixIdentity());

	XMStoreFloat4x4(&m_water->vsConstantBufferData.view, camera->getView());
	XMStoreFloat4x4(&m_water->vsConstantBufferData.model, XMMatrixTranslation(0.f, 1.f, 0.f));

	XMStoreFloat4x4(&m_bottom->vsConstantBufferData.view, camera->getView());
	XMStoreFloat4x4(&m_bottom->vsConstantBufferData.model, XMMatrixTranslation(0.f, -1.f, 0.f));
}

void XM_CALLCONV DirectXTK3DSceneRenderer::DrawGrid(FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	m_batchEffect->Apply(context);

	context->IASetInputLayout(m_batchInputLayout.Get());

	m_batch->Begin();

	xdivs = std::max<size_t>(1, xdivs);
	ydivs = std::max<size_t>(1, ydivs);

	for (size_t i = 0; i <= xdivs; ++i)
	{
		float fPercent = float(i) / float(xdivs);
		fPercent = (fPercent * 2.0f) - 1.0f;
		XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
		vScale = XMVectorAdd(vScale, origin);

		VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
		VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
		m_batch->DrawLine(v1, v2);
	}

	for (size_t i = 0; i <= ydivs; i++)
	{
		FLOAT fPercent = float(i) / float(ydivs);
		fPercent = (fPercent * 2.0f) - 1.0f;
		XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
		vScale = XMVectorAdd(vScale, origin);

		VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
		VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
		m_batch->DrawLine(v1, v2);
	}

	m_batch->End();
}

void DirectXTK3DSceneRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Set render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// Draw procedurally generated dynamic grid
	const XMVECTORF32 xaxis = { 20.f, 0.f, 0.f };
	const XMVECTORF32 yaxis = { 0.f, 0.f, 20.f };
	DrawGrid(xaxis, yaxis, g_XMZero, 20, 20, Colors::Gray);

	m_water->Draw(m_deviceResources);
	m_bottom->Draw(m_deviceResources);

	/*XMVECTOR qid = XMQuaternionIdentity();
	const XMVECTORF32 scale = { 1.f, 1.f, 1.f };
	const XMVECTORF32 translate = { 0.f, 0.f, 0.f };
	XMVECTOR rotate = XMQuaternionRotationRollPitchYaw(0, XM_PI / 2.f, -XM_PI / 2.f);
	XMMATRIX local = XMMatrixMultiply(world, XMMatrixTransformation(g_XMZero, qid, scale, g_XMZero, rotate, translate));
	m_model->Draw(context, *m_states, local, view, projection);*/
}

void DirectXTK3DSceneRenderer::ProcessInput(std::vector<PlayerInputData>* playerActions)
{
	camera->ProcessInput(playerActions);
}

void DirectXTK3DSceneRenderer::CreateDeviceDependentResources()
{
	// Create DirectXTK objects
	auto device = m_deviceResources->GetD3DDevice();
	m_states.reset(new CommonStates(device));

	auto fx = new EffectFactory(device);
	fx->SetDirectory(L"Assets");
	m_fxFactory.reset(fx);

	auto context = m_deviceResources->GetD3DDeviceContext();
	m_batch.reset(new PrimitiveBatch<VertexPositionColor>(context));

	m_batchEffect.reset(new BasicEffect(device));
	m_batchEffect->SetVertexColorEnabled(true);

	{
		void const* shaderByteCode;
		size_t byteCodeLength;

		m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

		DX::ThrowIfFailed(
			device->CreateInputLayout(VertexPositionColor::InputElements,
			VertexPositionColor::InputElementCount,
			shaderByteCode, byteCodeLength,
			m_batchInputLayout.ReleaseAndGetAddressOf())
			);
	}

	// Load textures
	DX::ThrowIfFailed(
		CreateDDSTextureFromFile(device, L"assets\\seafloor.dds", nullptr, m_texture1.ReleaseAndGetAddressOf())
		);

	//DX::ThrowIfFailed(
	//	CreateDDSTextureFromFile(device, L"assets\\windowslogo.dds", nullptr, m_texture2.ReleaseAndGetAddressOf())
	//	);

	// Load shaders asynchronously.
	auto loadWaterVSTask = DX::ReadDataAsync(L"WaterVertexShader.cso");
	auto loadWaterPSTask = DX::ReadDataAsync(L"WaterPixelShader.cso");

	auto createWaterVSTask = loadWaterVSTask.then([this](const std::vector<byte>& fileData) {
		m_water->LoadVS(m_deviceResources, fileData);
	});

	auto createWaterPSTask = loadWaterPSTask.then([this](const std::vector<byte>& fileData) {
		m_water->LoadPS(m_deviceResources, fileData);
	});

	auto loadBottomVSTask = DX::ReadDataAsync(L"BottomVertexShader.cso");
	auto loadBottomPSTask = DX::ReadDataAsync(L"BottomPixelShader.cso");

	auto createBottomVSTask = loadBottomVSTask.then([this](const std::vector<byte>& fileData) {
		m_bottom->LoadVS(m_deviceResources, fileData);
	});

	auto createBottomPSTask = loadBottomPSTask.then([this](const std::vector<byte>& fileData) {
		m_bottom->LoadPS(m_deviceResources, fileData);
	});

	(createWaterVSTask && createWaterPSTask &&
		createBottomVSTask && createBottomPSTask).then([this]() {
		m_loadingComplete = true;
	});
}

void DirectXTK3DSceneRenderer::ReleaseDeviceDependentResources()
{
	m_states.reset();
	m_fxFactory.reset();
	m_batch.reset();
	m_batchEffect.reset();
	m_model.reset();
	m_texture1.Reset();
	m_texture2.Reset();
	m_batchInputLayout.Reset();
}