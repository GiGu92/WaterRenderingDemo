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
	//m_water = std::shared_ptr<SceneObject>(new SceneObject(deviceResources, L"plane.cmo"));
	//m_bottom = std::shared_ptr<SceneObject>(new SceneObject(deviceResources, L"soccer.cmo"));
	m_water = std::shared_ptr<Plane>(new Plane(deviceResources, 500, 500, 0.1f, nullptr, L"assets//skybox.dds", L"assets//water_normal3.dds"));
	m_bottom = std::shared_ptr<Plane>(new Plane(deviceResources, 1, 1, 50.f, L"assets/seafloor.dds"));
	m_skybox = std::shared_ptr<SceneObject>(new SceneObject(deviceResources, L"sphere.cmo", L"assets//skybox.dds"));

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
	XMStoreFloat4x4(&m_skybox->vsConstantBufferData.projection, camera->getProjection());

	m_water->vsConstantBufferData.lightPos = XMFLOAT4(1000.f, 500.f, 0.f, 1.f);
	m_water->vsConstantBufferData.lightColor = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

}

void DirectXTK3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	camera->Update(timer);

	m_batchEffect->SetView(camera->getView());
	m_batchEffect->SetWorld(XMMatrixIdentity());

	XMStoreFloat4x4(&m_water->vsConstantBufferData.view, camera->getView());
	XMStoreFloat4x4(&m_water->vsConstantBufferData.model, XMMatrixScaling(10.f, 10.f, 10.f) * XMMatrixTranslation(0.f, 1.f, 0.f));
	XMStoreFloat4(&m_water->vsConstantBufferData.cameraPos, camera->getEye());
	m_water->vsConstantBufferData.totalTime = XMFLOAT4(timer.GetTotalSeconds(), 0, 0, 0);

	XMStoreFloat4x4(&m_bottom->vsConstantBufferData.view, camera->getView());
	XMStoreFloat4x4(&m_bottom->vsConstantBufferData.model, XMMatrixScaling(10.f, 10.f, 10.f) * XMMatrixTranslation(0.f, -5.f, 0.f));
	
	XMStoreFloat4x4(&m_skybox->vsConstantBufferData.view, camera->getView());
	XMStoreFloat4x4(&m_skybox->vsConstantBufferData.model, XMMatrixScaling(500.f, 500.f, 500.f) * XMMatrixTranslationFromVector(camera->getEye()));
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

	context->RSSetState(m_states->CullClockwise());
	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	m_skybox->Draw(m_deviceResources);

	context->RSSetState(m_states->CullCounterClockwise());

	// Draw procedurally generated dynamic grid
	const XMVECTORF32 xaxis = { 20.f, 0.f, 0.f };
	const XMVECTORF32 yaxis = { 0.f, 0.f, 20.f };
	//DrawGrid(xaxis, yaxis, g_XMZero, 20, 20, Colors::Gray);

	if (m_wireframe)
		context->RSSetState(m_states->Wireframe());

	m_bottom->Draw(m_deviceResources);

	context->OMSetBlendState(m_states->AlphaBlend(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthRead(), 0);
	m_water->Draw(m_deviceResources);

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

	auto loadSkyboxVSTask = DX::ReadDataAsync(L"SkyboxVertexShader.cso");
	auto loadSkyboxPSTask = DX::ReadDataAsync(L"SkyboxPixelShader.cso");

	auto createSkyboxVSTask = loadSkyboxVSTask.then([this](const std::vector<byte>& fileData) {
		m_skybox->LoadVS(m_deviceResources, fileData);
	});

	auto createSkyboxPSTask = loadSkyboxPSTask.then([this](const std::vector<byte>& fileData) {
		m_skybox->LoadPS(m_deviceResources, fileData);
	});

	(createWaterVSTask && createWaterPSTask &&
		createBottomVSTask && createBottomPSTask &&
		createSkyboxVSTask && createSkyboxPSTask).then([this]()
	{
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
	m_skyTexture.Reset();
	m_seaFloorTexture.Reset();
	m_batchInputLayout.Reset();
}