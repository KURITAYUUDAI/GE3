#pragma once
#include "myMath.h"

#include <vector>
#include <string>
#include <optional>
#include <functional>
#include <algorithm>
#include <cassert>

#include "PostEffectManager.h"
#include "Logger.h"

struct PostEffectHandle
{
    uint32_t value = 0;

    bool IsValid() const
    {
        return value != 0;
    }

    bool operator==(const PostEffectHandle& other) const
    {
        return value == other.value;
    }

    bool operator!=(const PostEffectHandle& other) const
    {
        return !(*this == other);
    }
};

inline constexpr PostEffectHandle kInvalidPostEffectHandle{ 0 };

class PostEffectController
{
public:
    using UpdateFunc = std::function<void(PostEffect&, float progress, float elapsedTime)>;

public:

    void Update(float deltaTime);

    void Cancel(PostEffectHandle handle);

    void CancelAll();

    bool IsActive(PostEffectHandle handle) const;

public:

    template<class EffectType>
    PostEffectHandle Emit(
        const std::string& effectName,
        std::optional<float> duration,
        std::function<void(EffectType&, float progress, float elapsedTime)> updater = nullptr,
        std::optional<int32_t> priority = std::nullopt);

    template<class EffectType>
    void Edit(
        PostEffectHandle handle,
        std::function<void(EffectType&)> editor);

    template<class EffectType>
    EffectType* Get(PostEffectHandle handle);

    bool SetPriority(
        PostEffectHandle handle,
        int32_t priority);

private:
    struct ActiveEffect
    {
        PostEffectHandle handle = kInvalidPostEffectHandle;

        std::string effectName;
        PostEffect* effect = nullptr;

        float elapsedTime = 0.0f;
        std::optional<float> duration;

        bool cancelRequested = false;

        UpdateFunc updateFunc;
    };

private:
    ActiveEffect* Find(PostEffectHandle handle);
    const ActiveEffect* Find(PostEffectHandle handle) const;

    void FlushCancelRequests();

private:
    PostEffectManager* postEffectManager_ = PostEffectManager::GetInstance();

    std::vector<ActiveEffect> activeEffects_;

    uint32_t nextHandleValue_ = 1;
};

template<class EffectType>
PostEffectHandle PostEffectController::Emit(
    const std::string& effectName,
    std::optional<float> duration,
    std::function<void(EffectType&, float progress, float elapsedTime)> updater, 
    std::optional<int32_t> priority)
{
    PostEffect* effect = postEffectManager_->Add(effectName, priority);
    assert(effect);

    auto* typedEffect = dynamic_cast<EffectType*>(effect);
    assert(typedEffect && "Emit: 指定したEffectTypeと生成されたPostEffectの型が一致していません。");

    PostEffectHandle handle{ nextHandleValue_++ };
    if (nextHandleValue_ == 0) nextHandleValue_ = 1;

    ActiveEffect active;
    active.handle = handle;
    active.effectName = effectName;
    active.effect = effect;
    active.duration = duration;

    if (updater)
    {
        active.updateFunc =
            [updater](PostEffect& baseEffect, float progress, float elapsedTime){
            auto& typed = static_cast<EffectType&>(baseEffect);
            updater(typed, progress, elapsedTime);
            };
    }

    activeEffects_.push_back(std::move(active));

#ifdef _DEBUG
    Logger::Log("PostEffectController::Emit -> handle=" + std::to_string(handle.value) +
    " effectPtr=" + std::to_string(reinterpret_cast<uintptr_t>(effect)) + " name=" + effectName + "\n");
#endif

    return handle;
}

template<class EffectType>
void PostEffectController::Edit(
    PostEffectHandle handle,
    std::function<void(EffectType&)> editor)
{
    ActiveEffect* active = Find(handle);

    if (!active || !active->effect || !editor)
    {
    #ifdef _DEBUG
        Logger::Log("PostEffectController::Edit early return: active=" + 
            std::to_string(reinterpret_cast<uintptr_t>(active)) + "\n");
    #endif
        return;
    }

    // manager が保持しているかチェック
    if (!postEffectManager_->HasEffect(active->effect))
    {
    #ifdef _DEBUG
        Logger::Log("PostEffectController::Edit WARNING: active.effect not found in manager ptr=" +
            std::to_string(reinterpret_cast<uintptr_t>(active->effect)) + "\n");
    #endif

        postEffectManager_->DebugPrintChain();
        // エフェクトが削除されている場合は early return
        return;
    }

#ifdef _DEBUG
    // dynamic_cast 前ログ
    Logger::Log("PostEffectController::Edit: attempting dynamic_cast for handle=" + 
        std::to_string(handle.value) +
        " effectPtr=" + std::to_string(reinterpret_cast<uintptr_t>(active->effect)) + "\n");
#endif

    auto* typedEffect = dynamic_cast<EffectType*>(active->effect);

    if (!typedEffect)
    {
    #ifdef _DEBUG
        Logger::Log("PostEffectController::Edit: dynamic_cast failed for handle=" + 
            std::to_string(handle.value) + "\n");
    #endif
        return;
    }

#ifdef _DEBUG
    Logger::Log("PostEffectController::Edit: dynamic_cast succeeded ptr=" + 
        std::to_string(reinterpret_cast<uintptr_t>(typedEffect)) + "\n");
#endif
    editor(*typedEffect);
}

template<class EffectType>
EffectType* PostEffectController::Get(PostEffectHandle handle)
{
    ActiveEffect* active = Find(handle);

    if (!active || !active->effect)
    {
        return nullptr;
    }

    return dynamic_cast<EffectType*>(active->effect);
}
