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
	this->water = std::shared_ptr<SceneObject>(new SceneObject());
	this->bottom = std::shared_ptr<SceneObject>(new SceneObject());
	this->water->indexCount = 0;
	this->bottom->indexCount = 0;
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}


// Initializes view parameters when the window size changes.
void WRDSceneRenderer::CreateWindowSizeDependentResources()
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

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&water->constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	XMStoreFloat4x4(
		&bottom->constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };


	XMStoreFloat4x4(&water->constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
	XMStoreFloat4x4(&bottom->constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void WRDSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);
	}

	XMMATRIX modelMatrix = XMMatrixTranslation(1, 0, 0);
	XMStoreFloat4x4(&water->constantBufferData.model, XMMatrixTranspose(modelMatrix));
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
	context->UpdateSubresource(
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
		);

	water->Draw(context);
	bottom->Draw(context);
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
		water->LoadMesh(m_deviceResources, L"");
	});

	auto loadBottomMeshTask = (createBottomVSTask && createBottomPSTask).then([this]() {
		bottom->LoadMesh(m_deviceResources, L"");
	});

	(loadWaterMeshTask && loadBottomMeshTask).then([this]() {
		m_loadingComplete = true;
	});
}

void WRDSceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
}