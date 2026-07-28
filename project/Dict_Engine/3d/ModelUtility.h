#pragma once
#include "DirectXBase.h"
#include "myMath.h"
#include "Quaternion.h"

struct VertexData
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
	Vector4 color;
};

struct Material
{
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	int32_t enableLighting = false;
	float shininess = 10.0f;
	float environmentCoefficient = 1.0f;
	float alphaReference = 0.0f;
	Matrix4x4 uvTransform = MakeIdentity4x4();
};

struct MeshGeometry
{
	std::vector<VertexData> vertices;	//!< 頂点データ
	std::vector<uint32_t> indices;	//!< インデックスデータ

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;	//<! バッファリソース
	VertexData* vertexData = nullptr;	//<! バッファポインタ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};	//<! バッファビュー

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;	//<! インデックスリソース
	uint32_t* indexData = nullptr;	//<! バッファリソース内のデータを指すポインタ
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};	//<! バッファリソースの使い道を補足するバッファビュー

	uint32_t materialIndex = 0;	 //!< マテリアル番号
};

struct MaterialAsset
{
	std::string name;
	Material material;
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};

//struct MaterialInstance
//{
//	Material material;
//	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;	//<! マテリアル用バッファリソース
//	Material* materialData = nullptr;	//<! バッファリソース内のデータを指すポインタ
//	std::string textureFilePath;
//	uint32_t textureIndex = 0;
//};

struct Node
{
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	std::string name;

	std::vector<uint32_t> meshIndices;
	std::vector<Node> children;
};

struct ModelData
{
	std::vector<MeshGeometry> meshes;
	std::vector<MaterialAsset> materialAssets;
	Node rootNode;
};


