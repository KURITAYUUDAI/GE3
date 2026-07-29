#pragma once
#include "PostEffect.h"
#include "../tool/effect/DissolveUtility.h"

class Dissolve : public PostEffect
{
public:
	void Initialize(uint32_t width, uint32_t height) override;

	void Finalize() override;

public: // 外部入出力

	std::vector<PassFunc> GetPasses() override;

	std::vector<std::vector<PassBarrier>> GetBarriers() override;

	void SetThreshold(const float threshold) { params_.threshold = threshold; }
	void SetEdgeColor(const Vector4 edgeColor) { params_.edgeColor = edgeColor; }

	const float& GetThreshold() { return params_.threshold; }
	const Vector4& GetEdgeColor() { return params_.edgeColor; }

	int32_t GetDefaultPriority() const override
	{
		return 1000;
	}

private:

	void Pass(D3D12_CPU_DESCRIPTOR_HANDLE destRTV, uint32_t srcSRVIndex);

	void RegisterPSOs();

private:

	DissolveParams  params_;

	std::string maskTextureFilePath_ = "Noise0.png";

	const std::string kPsoName_ = "Dissolve";
};

