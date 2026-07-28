#pragma once
#include "ModelUtility.h"

class MaterialInstance
{
public:

	void Initialize();

	void Draw(UINT RootParameterIndex, UINT SrvRootParameterIndex);

public:

	void SetMaterialAsset(const MaterialAsset& materialAsset);

	void SetColor(const Vector4& color){ materialData_->color = color; }
	void SetEnableLighting(const int32_t& enableLighting) { materialData_->enableLighting = enableLighting; }
	void SetUVTransform(const EulerTransform& uvTransform);
	void SetShininess(const float& shiniess) { materialData_->shininess = shiniess; }
	void SetEnvironmentCoefficient(const float& environmentCoefficient) { materialData_->environmentCoefficient = environmentCoefficient; }
	void SetAlphaReference(const float alphaReference){ materialData_->alphaReference = alphaReference; }

	const Vector4& GetColor() const { return materialData_->color; }
	const int32_t& GetEnableLighting() const { return materialData_->enableLighting; }
	const Matrix4x4& GetUVTransform() const { return materialData_->uvTransform; }
	const float& GetShininess() const { return materialData_->shininess; }
	const float& GetEnvironmentCoefficient() const { return materialData_->environmentCoefficient; }
	const float& GetAlphaReference() const { return materialData_->alphaReference; }

	
	void SetTexture(const std::string& directoryFilePath);

	void ResetTexture();

	const Material& GetMaterial() const { return *materialData_; }
	uint32_t GetTextureIndex() const { return textureIndex_; }

private:

	void CreateMaterialResource();

private:

	const MaterialAsset* materialAsset_ = nullptr;

	Material material_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	Material* materialData_ = nullptr;

	std::string textureFilePath_;
	uint32_t textureIndex_ = 0;
};

