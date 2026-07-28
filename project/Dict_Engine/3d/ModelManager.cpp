#include "ModelManager.h"
#include "DirectXBase.h"
#include "Model.h"

std::unique_ptr<ModelManager> ModelManager::instance_ = nullptr;

ModelManager* ModelManager::GetInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = std::make_unique<ModelManager>(ConstructorKey());
	}
	return instance_.get();
}

void ModelManager::Finalize()
{
	models_.clear();

	instance_.reset();

}

void ModelManager::LoadModel(const std::string& directoryPath, const std::string& filename)
{
	// 読み込み済みモデルを検索
	if (models_.contains(filename))
	{
		// 読み込み済みなら早期return
		return;
	}

	// モデルの生成とファイル読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize();
	model->LoadModelFile(directoryPath, filename);
	model->CreateResources();

	// モデルをmapコンテナに格納する
	models_.insert(std::make_pair(filename, std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filename)
{
	// 読み込み済みモデルを検索
	if (models_.contains(filename))
	{
		// 読み込み済みなら早期return
		return models_.at(filename).get();
	}

	// ファイル名一致無し
	return nullptr;
}

void ModelManager::CreateVoid(const std::string& name)
{
	// 読み込み済みモデルを検索
	if (models_.contains(name))
	{
		// 読み込み済みなら早期return
		return;
	}

	// モデルの生成とファイル読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize();

	// モデルをmapコンテナに格納する
	models_.insert(std::make_pair(name, std::move(model)));
}

void ModelManager::InsertMesh(const std::string& name, MeshGeometry newMesh)
{
	// 読み込み済みモデルを検索
	if (models_.contains(name))
	{
		// 読み込み済みなら早期return
		return;
	}

	// モデルの生成とファイル読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize();
	model->InsertMesh(newMesh);
	model->CreateResources();

	// モデルをmapコンテナに格納する
	models_.insert(std::make_pair(name, std::move(model)));

}

void ModelManager::CreateSphere(const std::string& materialPath)
{


}

void ModelManager::Initialize(DirectXBase* dxBase)
{
	dxBase_ = dxBase;
}
