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
	m_water = std::shared_ptr<SceneObject>(new SceneObject(deviceResources, L"plane.cmo"));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void DirectXTK3DSceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	camera = std::unique_ptr<Camera>(new Camera(
		XMFLOAT4(0.0f, 7.f, 15.f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),
		m_deviceResources));

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	/*XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMMATRIX projection = XMMatrixMultiply(perspectiveMatrix, orientationMatrix);*/

	m_batchEffect->SetProjection(camera->getProjection());

	XMStoreFloat4x4(&m_water->vsConstantBufferData.projection, camera->getProjection());
	//m_water->projection = projection;
	/*XMStoreFloat4x4(
		&m_water->constantBufferData.projection,
		projection
		);*/
}

void DirectXTK3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	/*XMVECTOR eye = XMVectorSet(0.0f, 7.f, 15.f, 0.0f);
	XMVECTOR at = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtRH(eye, at, up);*/
	//XMMATRIX world = XMMatrixRotationY(float(timer.GetTotalSeconds() * XM_PIDIV4));
	XMMATRIX world = /*XMMatrixRotationAxis(XMVectorSet(0,0,-1,0), XM_PIDIV2)*//* * XMMatrixScaling(100.f, 100.f, 100.f)*/XMMatrixIdentity();

	camera->Update(timer);

	m_batchEffect->SetView(camera->getView());
	m_batchEffect->SetWorld(XMMatrixIdentity());

	XMStoreFloat4x4(&m_water->vsConstantBufferData.view, camera->getView());
	XMStoreFloat4x4(&m_water->vsConstantBufferData.model, world);
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

	// Draw 3D object
	//XMMATRIX world = XMLoadFloat4x4(&m_world);
	//XMMATRIX view = XMLoadFloat4x4(&m_view);
	//XMMATRIX projection = XMLoadFloat4x4(&m_projection);

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

	m_model = Model::CreateFromCMO(device, L"plane.cmo", *m_fxFactory, false);

	// Load textures
	DX::ThrowIfFailed(
		CreateDDSTextureFromFile(device, L"assets\\seafloor.dds", nullptr, m_texture1.ReleaseAndGetAddressOf())
		);

	//DX::ThrowIfFailed(
	//	CreateDDSTextureFromFile(device, L"assets\\windowslogo.dds", nullptr, m_texture2.ReleaseAndGetAddressOf())
	//	);

	// Load shaders
	// Load shaders asynchronously.
	auto loadWaterVSTask = DX::ReadDataAsync(L"WaterVertexShader.cso");
	auto loadWaterPSTask = DX::ReadDataAsync(L"WaterPixelShader.cso");

	auto createWaterVSTask = loadWaterVSTask.then([this](const std::vector<byte>& fileData) {
		m_water->LoadVS(m_deviceResources, fileData);
	});

	auto createWaterPSTask = loadWaterPSTask.then([this](const std::vector<byte>& fileData) {
		m_water->LoadPS(m_deviceResources, fileData);
	});

	(createWaterVSTask && createWaterPSTask).then([this]() {
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