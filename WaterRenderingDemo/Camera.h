#pragma once
#include "DirectXHelpers.h"
#include "Helpers\DeviceResources.h"
#include "Helpers\InputManager.h"

using namespace DirectX;
using namespace WaterRenderingDemo;

class Camera
{
public:
	Camera();
	Camera(XMFLOAT4 eye, XMFLOAT4 at, XMFLOAT4 up, std::shared_ptr<DX::DeviceResources> deviceResources);
	
	inline XMVECTOR getEye() { return XMLoadFloat4(&eye); }
	inline XMVECTOR getAt() { return XMLoadFloat4(&at); }
	inline XMVECTOR getUp() { return XMLoadFloat4(&up); }
	inline XMVECTOR getDirection() { return XMVector3Normalize(getAt() - getEye()); }
	inline XMVECTOR getMovementDir() { return XMLoadFloat4(&movementDir); }
	inline void setMovementDir(XMVECTOR value) { XMStoreFloat4(&this->movementDir, value); }
	XMMATRIX getWorld();
	XMMATRIX getView();
	XMMATRIX getProjection();

	void ProcessInput(std::vector<PlayerInputData>* playerActions);
	void Update(DX::StepTimer const& timer);
	
	~Camera();

private:
	XMFLOAT4 eye;
	XMFLOAT4 at;
	XMFLOAT4 up;
	float fov;
	float aspectRatio;
	float nearClippingPane;
	float farClippingPane;
	
	float movementSpeed;
	XMFLOAT4 movementDir;

	XMFLOAT4X4 sceneOrientation;
};

