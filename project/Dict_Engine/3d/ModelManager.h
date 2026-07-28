#pragma once
#include <Windows.h>
#include <memory>
#include <map>
#include <string>


#include "DirectXBase.h"

#include "ModelUtility.h"

class Model;

class DirectXBase;

// モデルマネージャー
class ModelManager
{
public:
	// シングルトンインスタンスの取得
	static ModelManager* GetInstance();
	// 終了
	void Finalize();

	// コンストラクタに渡すための鍵
	class ConstructorKey
	{
	private:
		ConstructorKey() = default;
		friend class ModelManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit ModelManager(ConstructorKey){}

private:	// シングルトン化

	static std::unique_ptr<ModelManager> instance_;

	~ModelManager() = default;
	ModelManager(ModelManager&) = delete;
	ModelManager& operator=(ModelManager&) = delete;

	friend struct std::default_delete<ModelManager>;

public:
	// 初期化
	void Initialize(DirectXBase* dxBase);

	/// <summary>
	/// モデルファイルの読み込み
	/// </summary>
	/// <param name="filePath">モデルファイルのパス</param>
	void LoadModel(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// モデルの検索
	/// </summary>
	/// <param name="filePath">モデルファイルのパス</param>
	/// <returns>モデル</returns>
	Model* FindModel(const std::string& filename);

	void InsertMesh(const std::string& name, MeshGeometry newMesh);

	void CreateSphere(const std::string& materialPath);


public: // 外部入出力

	DirectXBase* GetDxBase() const { return dxBase_; }



private:	// 静的関数

	void CreateVoid(const std::string& name);

private:

	DirectXBase* dxBase_ = nullptr;

	// モデルデータ
	std::map<std::string, std::unique_ptr<Model>> models_;

};

