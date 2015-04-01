#pragma once

#include "..\Helpers\DeviceResources.h"
#include "Content\ShaderStructures.h"
#include "..\Helpers\StepTimer.h"
#include "SceneObject.h"
#include "Camera.h"

namespace WaterRenderingDemo
{
	class WRDSceneRenderer
	{
	public:
		WRDSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void ProcessInput(std::vector<PlayerInputData>* playerActions);
		void Update(DX::StepTimer const& timer);
		void Render();
		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return m_tracking; }


	private:
		void Rotate(float radians);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Camera
		std::unique_ptr<Camera> m_camera;

		// Direct3D resources for water geometry.
		std::shared_ptr<SceneObject> water;

		// Direct3D resources for bottom geometry.
		std::shared_ptr<SceneObject> bottom;

		// System resources for water geometry.
		ModelViewProjectionConstantBuffer    m_water_constantBufferData;
		uint32    m_water_indexCount;

		// System resources for bottom geometry.
		ModelViewProjectionConstantBuffer    m_bottom_constantBufferData;
		uint32    m_bottom_indexCount;

		// Variables used with the rendering loop.
		bool    m_loadingComplete;
		float   m_degreesPerSecond;
		bool    m_tracking;

		// Helper function for drawing multiple objects
		void DrawObject(
			Microsoft::WRL::ComPtr<ID3D11InputLayout>    inputLayout,
			Microsoft::WRL::ComPtr<ID3D11Buffer>         vertexBuffer,
			Microsoft::WRL::ComPtr<ID3D11Buffer>         indexBuffer,
			Microsoft::WRL::ComPtr<ID3D11VertexShader>   vertexShader,
			Microsoft::WRL::ComPtr<ID3D11PixelShader>    pixelShader,
			Microsoft::WRL::ComPtr<ID3D11Buffer>         constantBuffer,
			Microsoft::WRL::ComPtr<ID3D11DeviceContext2> context,
			uint32 indexCount);
	};
}
