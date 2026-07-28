#include "MaterialInstance.h"
#include "DirectXBase.h"
#include "SrvManager.h"
#include "TextureManager.h"


void MaterialInstance::Initialize()
{
	CreateMaterialResource();	
	*materialData_ = material_;
}

void MaterialInstance::SetUVTransform(const EulerTransform& uvTransform)
{
	Matrix4x4 uvTransformMatrix = MakeAffineMatrix(uvTransform.scale, uvTransform.rotate, uvTransform.translate);

	materialData_->uvTransform = uvTransformMatrix;
}

void MaterialInstance::SetTexture(const std::string& directoryFilePath)
{
	textureFilePath_ = directoryFilePath;
	// テクスチャファイル読み込み
	TextureManager::GetInstance()->LoadTexture(textureFilePath_);
	// 読み込んだテクスチャの番号を取得
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath_);
}

void MaterialInstance::ResetTexture()
{
	assert(materialAsset_);

	textureFilePath_ =
		materialAsset_->textureFilePath;

	textureIndex_ =
		materialAsset_->textureIndex;
}

void MaterialInstance::CreateMaterialResource()
{
	// マテリアルリソースを作成する。
	materialResource_ = DirectXBase::GetInstance()->CreateBufferResource(sizeof(Material));
	// MaterialResourceにデータを書き込むためのアドレスを取得してMaterialDataに割り当てる
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	assert(materialResource_);
}

void MaterialInstance::Draw(UINT RootParameterIndex, UINT SrvRootParameterIndex)
{
	assert(textureIndex_ != 0);

	ID3D12GraphicsCommandList* commandList = DirectXBase::GetInstance()->GetCommandList();

	// マテリアルのCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(RootParameterIndex, materialResource_->GetGPUVirtualAddress());

	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(SrvRootParameterIndex, textureIndex_);
}

void MaterialInstance::SetMaterialAsset(const MaterialAsset& materialAsset)
{
	materialAsset_ = &materialAsset;

	*materialData_ = materialAsset_->material;
	textureFilePath_ = materialAsset_->textureFilePath;
	textureIndex_ = materialAsset_->textureIndex;
}
