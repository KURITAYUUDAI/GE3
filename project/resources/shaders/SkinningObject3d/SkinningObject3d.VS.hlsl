#include "../Object3d/Object3d.hlsli"

// ===== [テキストより独自で変換したポイント] =======
// float32_t4 → float4
//                                         by ChatGPT
// ==================================================

struct TransformationMatrix
{
    row_major float4x4 WVP; // World View Projection matrixの略
    row_major float4x4 World;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct Well
{
	row_major float4x4 skeletonSpaceMatrix;
	row_major float4x4 skeletonSpaceInverseTransposeMatrix;
};
StructuredBuffer<Well> gMatrixPalette : register(t0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
	float4 weight : WEIGHT0;
	int4 index : INDEX0;
};

struct Skinned
{
	float4 position;
	float3 normal;
};

Skinned Skinning(VertexShaderInput input)
{
	Skinned skinned;
    
    // 位置の変換
	skinned.position = mul(input.position, gMatrixPalette[input.index.x].skeletonSpaceMatrix) * input.weight.x;
	skinned.position += mul(input.position, gMatrixPalette[input.index.y].skeletonSpaceMatrix) * input.weight.y;
	skinned.position += mul(input.position, gMatrixPalette[input.index.z].skeletonSpaceMatrix) * input.weight.z;
	skinned.position += mul(input.position, gMatrixPalette[input.index.w].skeletonSpaceMatrix) * input.weight.w;
	skinned.position.w = 1.0f;  // 確実に1を入れる
    
    // 法線の変換
	skinned.normal = mul(input.normal, (float3x3) gMatrixPalette[input.index.x].skeletonSpaceInverseTransposeMatrix) * input.weight.x;
	skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[input.index.y].skeletonSpaceInverseTransposeMatrix) * input.weight.y;
	skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[input.index.z].skeletonSpaceInverseTransposeMatrix) * input.weight.z;
	skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[input.index.w].skeletonSpaceInverseTransposeMatrix) * input.weight.w;
	skinned.normal = normalize(skinned.normal); // 正規化して戻してあげる
    
	return skinned;
}

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
	Skinned skinned = Skinning(input);	// まずSkinning計算を行って、Skinning後の頂点情報を手に入れる。ここでの頂点もSkeletonSpace
	// Skinning結果を使って変換
    output.position = mul(skinned.position, gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(skinned.normal, (float3x3) gTransformationMatrix.World));
    output.worldPosition = mul(skinned.position, gTransformationMatrix.World).xyz;
    output.color = input.color;
    return output;
}