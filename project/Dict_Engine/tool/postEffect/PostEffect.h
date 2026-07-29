#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include "myMath.h"

/// <summary>
/// ポストエフェクトの基底クラス
/// 
/// 継承して使う:
///   - Initialize() でRT・PSO等を初期化
///   - Draw()       で入力RTに処理を施し、自前の出力RTに書き込む
///   - 最終エフェクトのみ DrawFinal() でSwapChainに書き込む
/// </summary>
class PostEffect
{
public:
    template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    using PassFunc = std::function<void(D3D12_CPU_DESCRIPTOR_HANDLE destRTV, uint32_t srcSRVIndex)>;

    struct PassBarrier
    {
        ID3D12Resource* resource = nullptr;
        D3D12_RESOURCE_STATES before = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        D3D12_RESOURCE_STATES after = D3D12_RESOURCE_STATE_RENDER_TARGET;
    };


public:
    PostEffect() = default;
    virtual ~PostEffect() = default;

    // コピー禁止
    PostEffect(const PostEffect&) = delete;
    PostEffect& operator=(const PostEffect&) = delete;

    // -------------------------------------------------------
    //  純粋仮想関数（継承先で必ず実装する）
    // -------------------------------------------------------

    /// <summary>
    /// 初期化。出力RT・PSO・CBV等を生成する。
    /// PostEffectManagerのInitialize()から呼ばれる。
    /// </summary>
    virtual void Initialize(uint32_t width, uint32_t height) = 0;

    virtual void Update(){};

    /// <summary>
    /// エフェクトを適用して自前の出力RTに書き込む。
    /// 最終エフェクト以外はこちらを使う。
    /// </summary>
    /// <param name="srcResource">入力リソース</param>
    /// <param name="srcSRVIndex">入力リソースのSRVインデックス</param>
    /*
    virtual void Draw(ID3D12Resource* srcResource, uint32_t srcSRVIndex,
                     D3D12_CPU_DESCRIPTOR_HANDLE destRTV) = 0;*/

    virtual void Finalize() = 0;

    // 各パスの描画関数リストを返す
    // PostEffectManagerがこれを使ってピンポンバッファで順番に実行する
    virtual std::vector<PassFunc> GetPasses() = 0;

    // GetPasses() と対になるバリア情報を返す
    // 戻り値: パスごとのバリアリスト（書き込み前に張るもの）
	virtual std::vector<std::vector<PassBarrier>> GetBarriers() = 0;

    virtual int32_t GetDefaultPriority() const
    {
        return 0;
    }

    // -------------------------------------------------------
    //  エフェクト名（PostEffectManagerがAdd時に付与する）
    // -------------------------------------------------------
    const std::string& GetName() const { return name_; }
    void               SetName(const std::string& name) { name_ = name; }

public:
    // -------------------------------------------------------
    //  継承先から使う共通処理
    // -------------------------------------------------------


    /// <summary>
    /// リソースバリア（ステート遷移）ヘルパー
    /// </summary>
    static void TransitionResource(
        ID3D12GraphicsCommandList* cmdList,
        ID3D12Resource* resource,
        D3D12_RESOURCE_STATES      before,
        D3D12_RESOURCE_STATES      after);

protected:
    
    // テクスチャフォーマット（OffscreenRenderに合わせる）
    static const DXGI_FORMAT    kFormat_ = DXGI_FORMAT_R16G16B16A16_FLOAT;

    // クリアカラー
    static constexpr Vector4      kClearColor_ = { 0.0f, 0.0f, 0.0f, 1.0f };

private:
    std::string name_;
};


/*
① PostEffect を継承したクラスを作る
② Initialize() で CreateOutputBuffer() を呼ぶ（必須）
パラメータが必要なら定数バッファも作る
③ DrawInternal() に処理を書く
④ Draw()      → DrawInternal(srcResource, srcSRVIndex, outputRTV_)
DrawFinal() → DrawInternal(srcResource, srcSRVIndex, GetBackBufferRTVHandle())
⑤ Game::Initialize() で RegisterFactory() に登録する
*/
