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
	Camera(XMVECTOR eye, XMVECTOR at, XMVECTOR up, std::shared_ptr<DX::DeviceResources> deviceResources);
	
	inline XMVECTOR getEye() { return eye; }
	inline XMVECTOR getAt() { return at; }
	inline XMVECTOR getUp() { return up; }
	inline XMVECTOR getDirection() { return XMVector3Normalize(at - eye); }
	inline XMVECTOR getMovementDir() { return movementDir; }
	inline void setMovementDir(XMVECTOR value) { this->movementDir = value; }
	XMMATRIX getWorld();
	XMMATRIX getView();
	XMMATRIX getProjection();

	void ProcessInput(std::vector<PlayerInputData>* playerActions);
	void Update(DX::StepTimer const& timer);

	~Camera();

private:
	XMVECTOR eye;
	XMVECTOR at;
	XMVECTOR up;
	float fov;
	float aspectRatio;
	float nearClippingPane;
	float farClippingPane;
	
	float movementSpeed;
	XMVECTOR movementDir;

	XMMATRIX sceneOrientation;
};

