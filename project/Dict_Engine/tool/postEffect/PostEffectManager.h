#pragma once
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <cstdint>
#include "PostEffect.h"
#include "myMath.h"

#include <optional>

/// <summary>
/// ポストエフェクトを一括管理するシングルトンマネージャー
///
/// 使い方:
///   // 起動時（Game::Initialize）
///   PostEffectManager::GetInstance()->Initialize();
///
///   // エフェクトの種類を登録（起動時に1回）
///   PostEffectManager::GetInstance()->RegisterFactory(
///       "GaussianBlur", []{ return std::make_unique<GaussianBlur>(); });
///
///   // シーンのInitializeなどで適用順を設定
///   auto* pfxMgr = PostEffectManager::GetInstance();
///   pfxMgr->Clear();
///   pfxMgr->Add("Monochrome");
///   pfxMgr->Add("GaussianBlur");
///
///   // 毎フレーム DrawSwapChain() の後に呼ぶ
///   PostEffectManager::GetInstance()->Draw(
///       OffscreenRender::GetInstance()->GetRenderTextureResource(),
///       OffscreenRender::GetInstance()->GetSRVIndex());
/// </summary>
class PostEffectManager
{
public:
    template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    // エフェクトを生成するファクトリ関数の型
    using FactoryFunc = std::function<std::unique_ptr<PostEffect>()>;

    template<typename T>
    T* Get(const std::string& name)
    {
        for (auto& entry : effectChain_)
        {
            if (entry->GetName() == name)
            {
                return dynamic_cast<T*>(entry.effect.get());
            }
        }
        return nullptr; // 見つからなかった場合
    }

public:
    // シングルトン
    static PostEffectManager* GetInstance();
    void Finalize();

    class ConstructorKey
    {
    private:
        ConstructorKey() = default;
        friend class PostEffectManager;
    };
    explicit PostEffectManager(ConstructorKey){}

public:

    struct EffectEntry
    {
        std::unique_ptr<PostEffect> effect;
        int32_t priority = 0;
        uint64_t insertionOrder = 0;
    };

    struct PassEntry
    {
        PostEffect::PassFunc                    pass;
        std::vector<PostEffect::PassBarrier>    barriers; // このパス専用のバリア
    };

private:
    static std::unique_ptr<PostEffectManager> instance_;
    ~PostEffectManager() = default;
    PostEffectManager(const PostEffectManager&) = delete;
    PostEffectManager& operator=(const PostEffectManager&) = delete;
    friend struct std::default_delete<PostEffectManager>;

public:
    // -------------------------------------------------------
    //  初期化 / 終了
    // -------------------------------------------------------

    /// <summary>
    /// マネージャーの初期化。Game::Initialize() から呼ぶ。
    /// </summary>
    /// <param name="width">レンダーターゲットの幅</param>
    /// <param name="height">レンダーターゲットの高さ</param>
    void Initialize(uint32_t width, uint32_t height);

    // -------------------------------------------------------
    //  ファクトリ登録
    // -------------------------------------------------------

    /// <summary>
    /// エフェクトの生成関数を名前で登録する。
    /// Game::Initialize() でまとめて登録しておく。
    /// </summary>
    /// <param name="name">エフェクト名（Add()で使う文字列）</param>
    /// <param name="factory">エフェクトを生成するラムダ</param>
    void RegisterFactory(const std::string& name, FactoryFunc factory);

    // -------------------------------------------------------
    //  エフェクトチェーンの構築（Scene::Initialize等から呼ぶ）
    // -------------------------------------------------------

   

    /// <summary>
    /// エフェクトをチェーンの末尾に追加する。
    /// 追加した順番がそのまま適用順になる。
    /// </summary>
    /// <param name="name">RegisterFactory()で登録した名前</param>
    PostEffect* Add(const std::string& name,
        std::optional<int32_t> priority = std::nullopt);

    /// <summary>
    /// 指定インデックスのエフェクトをチェーンから取り除く。
    /// </summary>
    void Remove(uint32_t index);

    void Remove(const std::string& name);

    void Remove(PostEffect* target);

    /// <summary>
    /// エフェクトの順番を入れ替える。
    /// </summary>
    void Swap(uint32_t indexA, uint32_t indexB);

    bool HasEffect(const PostEffect* target) const;
    void DebugPrintChain() const;

    // -------------------------------------------------------
    //  毎フレームの描画
    // -------------------------------------------------------

    /// <summary>
    /// チェーン順にエフェクトを適用し、最終結果をSwapChainに転写する。
    /// DrawSwapChain() の後、ImGui::Draw() の前に呼ぶ。
    /// エフェクトが0個の場合はそのままSwapChainにコピーする。
    /// </summary>
    /// <param name="srcResource">シーンRTのリソース</param>
    /// <param name="srcSRVIndex">シーンRTのSRVインデックス</param>
    void Draw(ID3D12Resource* srcResource, uint32_t srcSRVIndex);

    // -------------------------------------------------------
    //  デバッグ・情報取得
    // -------------------------------------------------------

    /// <summary>現在のチェーンに含まれるエフェクト数</summary>
    uint32_t GetEffectCount() const { return static_cast<uint32_t>(effectChain_.size()); }

    /// <summary>指定インデックスのエフェクト名を返す</summary>
    const std::string& GetEffectName(uint32_t index) const;

    bool SetPriority(
     PostEffect* effect,
     const int32_t& priority);

    


private:

    // -------------------------------------------------------
    //  内部処理
    // -------------------------------------------------------

    /// <summary>
    /// エフェクトが0個のときのフォールバック描画（シーンRTをそのままSwapChainへ）
    /// </summary>
    void DrawPassthrough(ID3D12Resource* srcResource, uint32_t srcSRVIndex);
    void CreatePingPongBuffers(uint32_t width, uint32_t height);
    void RegisterPassthroughPSO();
    void SortEffectChain();

    /// <summary>
   /// 現在のエフェクトチェーンをすべてクリアする。
   /// シーン遷移時に呼ぶ。
   /// </summary>
    void Clear();

public: // 外部入出力

	const PostEffect* GetEffect(uint32_t index) const
	{
		assert(index < effectChain_.size() && "PostEffectManager: GetEffect インデックスが範囲外です");
		return effectChain_[index].effect.get();
	}

private:
    // 登録されたファクトリ関数（名前 → 生成関数）
    std::unordered_map<std::string, FactoryFunc> factories_;

    // 現在のエフェクトチェーン（適用順）
    std::vector<EffectEntry> effectChain_;
    uint64_t nextInsertionOrder_ = 0;

    // ピンポンバッファ（2本固定）
    ComPtr<ID3D12Resource>      pingPongRT_[2];
    D3D12_CPU_DESCRIPTOR_HANDLE pingPongRTV_[2] = {};
    uint32_t                    pingPongRTVIndex_[2] = {};
    uint32_t                    pingPongSRVIndex_[2] = {};

    // 初期化時のRT解像度（Add()時のInitializeに渡す）
    uint32_t width_ = 0;
    uint32_t height_ = 0;

    // フォールバック用PSOの名前
    const std::string kPsoNamePassthrough_ = "PostEffectPassthrough";

    bool isReady_ = false;
};


