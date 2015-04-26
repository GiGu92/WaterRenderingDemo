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
	SceneObject(std::shared_ptr<DX::DeviceResources> deviceResources, const wchar_t* modelFile, const wchar_t* diffuseTextureFile = nullptr);
	
	void Draw(std::shared_ptr<DX::DeviceResources> deviceResources);
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext2> context);

	void LoadVS(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		const std::vector<byte>& vsFileData);
	void LoadPS(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		const std::vector<byte>& psFileData);
	void LoadMesh(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		const wchar_t* modelFile);
	
	~SceneObject();

//private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>         vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>          pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>               vsConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>               psConstantBuffer;
	ModelViewProjectionConstantBuffer                  vsConstantBufferData;
	ModelViewProjectionConstantBuffer                  psConstantBufferData;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>          inputLayout;
	std::shared_ptr<CommonStates>                      states;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>   diffuseTexture;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>         linearSampler;

	std::unique_ptr<DirectX::Model> model;
};

