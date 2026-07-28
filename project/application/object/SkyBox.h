#pragma once
#include "DirectXBase.h"
#include "object3d.h"
#include "myMath.h"
#include "Model.h"
#include "ModelUtility.h"
#include "MaterialInstance.h"

class SkyBox
{
public:
	/*struct TransformationMatrix
	{
		Matrix4x4 WVP;
		Matrix4x4 World;
	};*/

public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

public:
	void SetCamera(Camera* camera){ camera_ = camera; }

	uint32_t GetEnvironmentTextureIndex() const;
private:

	// VertexResourceを作成
	void CreateVertexResource();

	// IndexResourceを作成
	void CreateIndexResource();

	ModelData CreateSkyBox();

private:
	std::string psoName_ = "SkyBox";
	PSOManager::BlendMode blendMode_ = PSOManager::BlendMode::Normal;
	PSOManager::FillMode fillMode_ = PSOManager::FillMode::kSolid;

	Camera* camera_;
	std::unique_ptr<ModelData> modelData_;
	std::unique_ptr<MaterialInstance> material_;

	WorldTransform worldTransform_;
};