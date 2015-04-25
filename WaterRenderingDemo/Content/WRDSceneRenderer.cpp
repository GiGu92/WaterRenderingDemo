#include "pch.h"
#include "WRDSceneRenderer.h"

#include "..\Helpers\DirectXHelper.h"

using namespace WaterRenderingDemo;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the scene geometry.
WRDSceneRenderer::WRDSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	//this->water = std::shared_ptr<SceneObject>(new SceneObject());
	this->water = std::shared_ptr<SceneObject>(new SceneObject(m_deviceResources, L"plane.cmo"));
	this->bottom = std::shared_ptr<SceneObject>(new SceneObject());
	this->water->indexCount = 0;
	this->bottom->indexCount = 0;
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}


// Initializes view parameters when the window size changes.
void WRDSceneRenderer::CreateWindowSizeDependentResources()
{
	// Camera
	m_camera = std::unique_ptr<Camera>(new Camera(
		XMVectorSet(2.0f, 1.0f, -5.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
		m_deviceResources));

	XMStoreFloat4x4(&water->constantBufferData.model, XMMatrixTranspose(XMMatrixScaling(100,100,100)));
	XMStoreFloat4x4(&bottom->constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(0.0f, 0.0f, 0.0f)));

	XMStoreFloat4x4(&water->constantBufferData.projection, m_camera->getProjection());
	XMStoreFloat4x4(&bottom->constantBufferData.projection, m_camera->getProjection());

	XMStoreFloat4x4(&water->constantBufferData.view, m_camera->getView());
	XMStoreFloat4x4(&bottom->constantBufferData.view, m_camera->getView());
}

void WRDSceneRenderer::ProcessInput(std::vector<PlayerInputData>* playerActions)
{
	m_camera->ProcessInput(playerActions);
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void WRDSceneRenderer::Update(DX::StepTimer const& timer)
{
	m_camera->Update(timer);

	XMStoreFloat4x4(&water->constantBufferData.view, m_camera->getView());
	XMStoreFloat4x4(&bottom->constantBufferData.view, m_camera->getView());

	XMStoreFloat4x4(&water->constantBufferData.projection, m_camera->getProjection()	);
	XMStoreFloat4x4(&bottom->constantBufferData.projection, m_camera->getProjection());

	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		//double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		double totalRotation = 0;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);
	}

	XMMATRIX modelMatrix = XMMatrixTranslation(1, 0, 0);
	//XMStoreFloat4x4(&water->constantBufferData.model, XMMatrixTranspose(modelMatrix));
}

// Rotate the 3D cube model a set amount of radians.
void WRDSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&water->constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
	XMStoreFloat4x4(&bottom->constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void WRDSceneRenderer::StartTracking()
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void WRDSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void WRDSceneRenderer::StopTracking()
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void WRDSceneRenderer::Render()
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

	// Prepare the constant buffers to send it to the graphics device.
	/*context->UpdateSubresource(
		water->constantBuffer.Get(),
		0,
		NULL,
		&water->constantBufferData,
		0,
		0
		);
	context->UpdateSubresource(
		bottom->constantBuffer.Get(),
		0,
		NULL,
		&bottom->constantBufferData,
		0,
		0
		);*/

	//water->Draw(context);
	//bottom->Draw(context);

	water->Draw(m_deviceResources);
}

void WRDSceneRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadWaterVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadWaterPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	auto loadBottomVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadBottomPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	auto createWaterVSTask = loadWaterVSTask.then([this](const std::vector<byte>& fileData) {
		water->LoadVS(m_deviceResources, fileData);
	});

	auto createWaterPSTask = loadWaterPSTask.then([this](const std::vector<byte>& fileData) {
		water->LoadPS(m_deviceResources, fileData);
	});

	auto createBottomVSTask = loadBottomVSTask.then([this](const std::vector<byte>& fileData) {
		bottom->LoadVS(m_deviceResources, fileData);
	});

	auto createBottomPSTask = loadBottomPSTask.then([this](const std::vector<byte>& fileData) {
		bottom->LoadPS(m_deviceResources, fileData);
	});

	auto loadWaterMeshTask = (createWaterVSTask && createWaterPSTask).then([this]() {
		water->LoadCubeMesh(m_deviceResources, L"");
	});

	auto loadBottomMeshTask = (createBottomVSTask && createBottomPSTask).then([this]() {
		bottom->LoadCubeMesh(m_deviceResources, L"");
	});

	(loadWaterMeshTask && loadBottomMeshTask).then([this]() {
		m_loadingComplete = true;
	});
}

void WRDSceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
}