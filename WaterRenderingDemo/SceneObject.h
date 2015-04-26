#pragma once

#include "Helpers\DeviceResources.h"
#include "Content\ShaderStructures.h"

#include <Model.h>
#include <Effects.h>
#include <CommonStates.h>

using namespace WaterRenderingDemo;
using namespace DirectX;

class SceneObject
{
	//friend class ::WRDSceneRenderer;

public:
	SceneObject();
	SceneObject(std::shared_ptr<DX::DeviceResources> deviceResources, const wchar_t* modelFile, bool alpha = false);
	
	void Draw(std::shared_ptr<DX::DeviceResources> deviceResources);
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext2> context);

	void LoadVS(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		const std::vector<byte>& vsFileData);
	void LoadPS(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		const std::vector<byte>& psFileData);
	void LoadCubeMesh(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		const wchar_t* modelFile);
	void LoadPlaneMesh(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		const wchar_t* modelFile);
	void LoadMesh(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		const wchar_t* modelFile);
	
	~SceneObject();

//private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>   vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>    pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>         vsConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>         psConstantBuffer;
	ModelViewProjectionConstantBuffer            vsConstantBufferData;
	ModelViewProjectionConstantBuffer            psConstantBufferData;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>    inputLayout;
	std::shared_ptr<CommonStates>                states;

	std::unique_ptr<DirectX::Model> model;
	bool alpha;
};

