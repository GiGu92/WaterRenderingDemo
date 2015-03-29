#pragma once

#include "Helpers\DeviceResources.h"
#include "Content\ShaderStructures.h"

#include <Model.h>
#include <Effects.h>
#include <CommonStates.h>

using namespace WaterRenderingDemo;

class SceneObject
{
	//friend class ::WRDSceneRenderer;

public:
	SceneObject();
	SceneObject(std::shared_ptr<DX::DeviceResources> deviceResources, const wchar_t* modelFile);
	SceneObject(
		Microsoft::WRL::ComPtr<ID3D11InputLayout>    inputLayout,
		Microsoft::WRL::ComPtr<ID3D11Buffer>         vertexBuffer,
		Microsoft::WRL::ComPtr<ID3D11Buffer>         indexBuffer,
		Microsoft::WRL::ComPtr<ID3D11VertexShader>   vertexShader,
		Microsoft::WRL::ComPtr<ID3D11PixelShader>    pixelShader,
		Microsoft::WRL::ComPtr<ID3D11Buffer>         constantBuffer,
		ModelViewProjectionConstantBuffer            constantBufferData,
		uint32 indexCount);
	
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
	Microsoft::WRL::ComPtr<ID3D11InputLayout>    inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>         vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>         indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>   vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>    pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>         constantBuffer;
	ModelViewProjectionConstantBuffer            constantBufferData;
	uint32 indexCount;

	std::unique_ptr<DirectX::Model> model;
};

