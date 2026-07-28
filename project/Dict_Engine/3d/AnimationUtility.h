#pragma once
#include "DirectXBase.h"
#include "myMath.h"
#include "Quaternion.h"

#include "ModelUtility.h"
#include "SplineCurve.h"

#include <map>
#include <cassert>
#include <optional>

template <typename tValue>
struct Keyframe
{
	float time;	//!< キーフレームの値
	tValue value;	//!< キーフレームの時刻（単位は秒）
};

using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

template<typename tValue>
struct AnimationCurve
{
	std::vector<Keyframe<tValue>> keyframes;
};

struct NodeAnimation
{
	AnimationCurve<Vector3> translate;
	AnimationCurve<Quaternion> rotate;
	AnimationCurve<Vector3> scale;
};

struct Animation
{
	float duration;	// アニメーション全体の尺（単位は秒）
	// NodeAnimationの集合。Node名でひけるようにしておく
	std::map<std::string, NodeAnimation> nodeAnimations;
};

Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename);

template<typename T>
T LerpAnimation(const T& start, const T& end, float t);

// Vector3用の特殊化
template<>
inline Vector3 LerpAnimation<Vector3>(const Vector3& start, const Vector3& end, float t) 
{
	return Lerp(start, end, t);
}

// Quaternion用の特殊化
template<>
inline Quaternion LerpAnimation<Quaternion>(const Quaternion& start, const Quaternion& end, float t) 
{
	return Slerp(start, end, t);
}

template<typename tValue>
tValue CalculateValue(const AnimationCurve<tValue>& aCurve, float time)
{
	assert(!aCurve.keyframes.empty());	// キーがないものは返す値がわからないのでダメ
	if (aCurve.keyframes.size() == 1 || time <= aCurve.keyframes[0].time)	// キーが1つか、時刻がキーフレーム前なら最初の値とする
	{
		return aCurve.keyframes[0].value;
	}

	for (size_t index = 0; index < aCurve.keyframes.size() - 1; ++index)
	{
		size_t nextIndex = index + 1;
		// indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
		if (aCurve.keyframes[index].time <= time && time <= aCurve.keyframes[nextIndex].time)
		{
			// 範囲内を補完する
			float t = (time - aCurve.keyframes[index].time) 
				/ (aCurve.keyframes[nextIndex].time - aCurve.keyframes[index].time);
			return LerpAnimation(aCurve.keyframes[index].value, aCurve.keyframes[nextIndex].value, t);
		}
	}

	// ここまで着た場合は一番後の時刻よりも後ろなので最後の値を返すことにする
	return aCurve.keyframes.rbegin()->value;
}

Matrix4x4 PlayAnimation(Node rootNode, Animation& animation, float& animationTime, const float& DeltaTime);


struct Joint
{
	QuaternionTransform transform;	// Tranform情報
	Matrix4x4 localMatrix;	// localMatrix
	Matrix4x4 skeletonSpaceMatrix;	// skeletonSpaceでの変換行列
	std::string name;	// 名前
	std::vector<int32_t> children;	// 子Jointのリスト。いなければ空
	int32_t index;	// 自身のIndex
	std::optional<int32_t> parent;	// 親JointのIndex。いなければnull
};

struct Skeleton
{
	int32_t root;	// RootJointのIndex
	std::map<std::string, int32_t> joinMap;	// Joint名とIndexとの辞書
	std::vector<Joint> joints;	// 所属しているジョイント
};

void UpdateSkeleton(Skeleton& skeleton);

Skeleton CreateSkeleton(const Node& rootNode);

int32_t CreateJoint(
	const Node& node,
	const std::optional<int32_t>& parent,
	std::vector<Joint>& joints);

void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime);

void DrawDebug(Skeleton& skeleton, const Matrix4x4& worldMatrix);

void ImGuiDebug(Skeleton& skeleton);







