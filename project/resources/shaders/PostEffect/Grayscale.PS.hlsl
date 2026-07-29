#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// ===== [テキストより独自で変換したポイント] =======
// float32_t4 → float4
//                                         by ChatGPT
// ==================================================

struct Material
{
	float intensity;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output; 
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    float value = dot(output.color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
	float3 grayscale = float3(value, value, value);
    
	output.color.rgb = lerp(output.color.rgb, grayscale.rgb, gMaterial.intensity);
    
    return output;
}