#include "pch.h"
#include "Camera.h"

using namespace WaterRenderingDemo;

Camera::Camera()
{
	this->eye = XMVectorSet( 0.0f, 0.7f, 10.5f, 0.0f );
	this->at = XMVectorSet(0.0f, -0.1f, 0.0f, 0.0f);
	this->up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
};

Camera::Camera(XMVECTOR eye, XMVECTOR at, XMVECTOR up,
	std::shared_ptr<DX::DeviceResources> deviceResources)
	: eye(eye), at(at), up(up)
{
	Windows::Foundation::Size outputSize = deviceResources->GetOutputSize();
	this->aspectRatio = outputSize.Width / outputSize.Height;
	this->fov = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		this->fov *= 2.0f;
	}

	this->nearClippingPane = 0.01f;
	this->farClippingPane = 100.0f;

	XMFLOAT4X4 orientation = deviceResources->GetOrientationTransform3D();
	this->sceneOrientation = XMLoadFloat4x4(&orientation);

	this->movementSpeed = 5.0f;
	this->movementDir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
}

XMMATRIX Camera::getWorld()
{
	return XMMatrixTranspose(XMMatrixTranslationFromVector(eye));
}

XMMATRIX Camera::getView()
{
	return XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up));
}

XMMATRIX Camera::getProjection()
{
	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fov,
		aspectRatio,
		nearClippingPane,
		farClippingPane);

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.
	return XMMatrixTranspose(perspectiveMatrix * sceneOrientation);
}

void Camera::ProcessInput(std::vector<PlayerInputData>* playerActions)
{
	for (int i = 0; i < playerActions->size(); i++)
	{
		PlayerInputData playerAction = (*playerActions)[i];

		XMVECTOR md = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		if (playerAction.PlayerAction == PLAYER_ACTION_TYPES::INPUT_MOVE)
		{
			XMVECTOR forward = this->getDirection();
			XMVECTOR right = XMVector3Cross(forward, this->up);
			md += playerAction.Y * forward + playerAction.X * right;
		}

		if (playerAction.PlayerAction == PLAYER_ACTION_TYPES::INPUT_JUMP_DOWN)
		{
			md += this->up;
		}

		if (playerAction.PlayerAction == PLAYER_ACTION_TYPES::INPUT_DUCK_DOWN)
		{
			md += -this->up;
		}

		this->setMovementDir(md);
	}
}

void Camera::Update(DX::StepTimer const& timer)
{
	this->eye += this->movementDir * this->movementSpeed * timer.GetElapsedSeconds();
}

Camera::~Camera()
{
}
