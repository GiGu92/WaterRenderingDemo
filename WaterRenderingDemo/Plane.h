#pragma once
#include "SceneObject.h"

struct VertexPositionNormalTextureTangentBinormal
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 textureCoordinate;
	XMFLOAT3 tangent;
	XMFLOAT3 binormal;

	VertexPositionNormalTextureTangentBinormal()
	{ }

	VertexPositionNormalTextureTangentBinormal(XMFLOAT3 const& position, XMFLOAT3 const& normal, XMFLOAT2 const& textureCoordinate, XMFLOAT3 const& tangent, XMFLOAT3 const& binormal)
		: position(position),
		normal(normal),
		textureCoordinate(textureCoordinate),
		tangent(tangent),
		binormal(binormal)
	{ }
};

class Plane : public SceneObject
{
public:
	Plane(std::shared_ptr<DX::DeviceResources> deviceResources,
		int width = 5, int height = 5, float stride = 1.f, 
		const wchar_t* diffuseTextureFile = nullptr,
		const wchar_t* environmentTextureFile = nullptr,
		const wchar_t* normalTextureFile = nullptr);
	void GenerateMesh(std::shared_ptr<DX::DeviceResources> deviceResources);
	virtual void Draw(std::shared_ptr<DX::DeviceResources> deviceResources);
	virtual void LoadVS(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		const std::vector<byte>& vsFileData);
	~Plane();
private:
	int width;
	int height;
	float stride;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	int indexCount;
};

