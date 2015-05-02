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
	SceneObject(std::shared_ptr<DX::DeviceResources> deviceResources, 
		const wchar_t* modelFile, 
		const wchar_t* diffuseTextureFile = nullptr,
		const wchar_t* environmentTextureFile = nullptr,
		const wchar_t* normalTextureFile = nullptr);
	
	virtual void Draw(std::shared_ptr<DX::DeviceResources> deviceResources);

	virtual void LoadVS(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		const std::vector<byte>& vsFileData);
	virtual void LoadPS(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		const std::vector<byte>& psFileData);
	virtual void LoadMesh(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		const wchar_t* modelFile);
	
	~SceneObject();

//private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>         vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>          pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>               vsConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>               psConstantBuffer;
	MyConstantBuffer                                   vsConstantBufferData;
	MyConstantBuffer                                   psConstantBufferData;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>          inputLayout;
	std::shared_ptr<CommonStates>                      states;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>   diffuseTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>   environmentTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>   normalTexture;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>         linearSampler;

	std::unique_ptr<DirectX::Model> model;
};

