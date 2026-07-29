#pragma once
#include "PostEffect.h"

class Grayscale : public PostEffect
{
public:

    struct GrayscaleParam
    {
        float intensity = 1.0f;
    };

public:
    void Initialize(uint32_t width, uint32_t height) override;

    void Finalize() override;


public: // 外部入出力

    std::vector<PassFunc> GetPasses() override;
    std::vector<std::vector<PassBarrier>> GetBarriers() override;

    void SetIntensity(const float& intensity) { param_.intensity = intensity; }
    
    const float& GetIntensity() const { return param_.intensity; }
    
private:

    void Pass(uint32_t srcSRVIndex, D3D12_CPU_DESCRIPTOR_HANDLE destRTV);

    void CreateConstantBuffer();
    void RegisterPSOs();
    void UpdateConstantBuffer();

private:

    ComPtr<ID3D12Resource> constantBufferResource_;
    GrayscaleParam* constantBufferMappedData_ = nullptr;
    GrayscaleParam param_;

    const std::string kPsoName_ = "Grayscale";
};

