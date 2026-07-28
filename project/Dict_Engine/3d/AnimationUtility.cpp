#include "AnimationUtility.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "ResourcePath.h"

#include "DebugDrawManager.h"

#include "ImGuiManager.h"

Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename)
{
	Animation animation;	// 今回作るアニメーション
	Assimp::Importer importer;

	std::string relativePath = directoryPath + "/" + filename;
	std::string fullPath = ResourcePath::MakeString(relativePath);

	const aiScene* scene = importer.ReadFile(fullPath.c_str(), 0);
	assert(scene->mNumAnimations != 0);	// アニメーションがない
	aiAnimation* animationAssimp = scene->mAnimations[0];	// 最初のアニメーションだけ採用。
	animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);	// 時間の単位を秒に変換

	// assimpでは個々のNodeのAnimationをchannelと読んでいるのでchannelを回してNoadAnimationの情報を取ってくる
	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex)
	{
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex)
		{
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);	// ここも秒に変換
			keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };	// 右手→左手
			nodeAnimation.translate.keyframes.push_back(keyframe);
		}

		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex)
		{
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);	// ここも秒に変換
			keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y,
							  -keyAssimp.mValue.z,  keyAssimp.mValue.w };	// 右手→左手
			nodeAnimation.rotate.keyframes.push_back(keyframe);
		}

		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex)
		{
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);	// ここも秒に変換
			keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };	// 右手→左手
			nodeAnimation.scale.keyframes.push_back(keyframe);
		}
	}

	// 解析完了
	return animation;
}

Matrix4x4 PlayAnimation(Node rootNode, Animation& animation, float& animationTime, const float& DeltaTime)
{
	NodeAnimation& rootNodeAnimation = animation.nodeAnimations[rootNode.name];
	Vector3	translate = CalculateValue<Vector3>(rootNodeAnimation.translate, animationTime);
	Quaternion rotate = CalculateValue<Quaternion>(rootNodeAnimation.rotate, animationTime);
	Vector3		scale = CalculateValue<Vector3>(rootNodeAnimation.scale, animationTime);
	Matrix4x4 localMatrix = MakeAffineMatrix(scale, rotate, translate);

	return localMatrix;
}

void UpdateSkeleton(Skeleton& skeleton)
{
	// すべてのJointを更新。親が若いので通常ループで処理可能になっている
	for (Joint& joint : skeleton.joints)
	{
		joint.localMatrix = 
			MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		if (joint.parent)	// 親がいれば親の行列を掛ける
		{
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
		}
		else	// 親がいないのでlocalMatrixとSkeletonSpaceMatrixは一致する
		{
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}


Skeleton CreateSkeleton(const Node& rootNode)
{
	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	// 名前とindexのマッピングを行いアクセスしやすくなる
	for (const Joint& joint : skeleton.joints)
	{
		skeleton.joinMap.emplace(joint.name, joint.index);
	}

	UpdateSkeleton(skeleton);

	return skeleton;
}

int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints)
{
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = int32_t(joints.size());	// 現在登録されてる数をIndexに
	joint.parent = parent;
	joints.push_back(joint);	// SkeletonのJoint列に登録
	for (const Node& child : node.children)
	{
		// 子Jointを作成し、そのIndexを登録
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}

	// 自身のIndexを返す
	return joint.index;
}

void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime)
{
	for (Joint& joint : skeleton.joints)
	{
		// 対象のJointのAnimationがあれば、値の適用を行う。下記のif文はC++17から可能になった初期化付きif文
		if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end())
		{
			const NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.translate = CalculateValue(rootNodeAnimation.translate, animationTime);
			joint.transform.rotate = CalculateValue(rootNodeAnimation.rotate, animationTime);
			joint.transform.scale = CalculateValue(rootNodeAnimation.scale, animationTime);
		}
	}
}

void DrawDebug(Skeleton& skeleton, const Matrix4x4& worldMatrix)
{
	DebugDrawManager* debugDrawManager = DebugDrawManager::GetInstance();

	// すべてのJointを描画。
	for (Joint& joint : skeleton.joints)
	{
		Vector3 worldTranslate;
		worldTranslate = GetWorldPosition(joint.skeletonSpaceMatrix * worldMatrix);

		debugDrawManager->AddSphere(worldTranslate, 0.01f, {1.0f, 1.0f, 1.0f, 1.0f}, 5, 5);

		if (joint.parent)
		{
			Vector3 parentWorldTranslate;
			parentWorldTranslate = GetWorldPosition(
				skeleton.joints[*joint.parent].skeletonSpaceMatrix * worldMatrix);

			debugDrawManager->AddLine(worldTranslate, parentWorldTranslate, { 1.0f, 1.0f, 1.0f, 1.0f });
		} 
		else
		{
			Vector3 skeletonWorldTranslate = GetWorldPosition(worldMatrix);
			debugDrawManager->AddLine(worldTranslate, skeletonWorldTranslate, { 1.0f, 1.0f, 1.0f, 1.0f });
		}
	}
}

void ImGuiDebug(Skeleton& skeleton)
{
	// すべてのJointを描画。
	for (Joint& joint : skeleton.joints)
	{
		
	}
}
