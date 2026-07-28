#define NOMINMAX
#include "DebugDrawManager.h"
#include <algorithm>
#include "Logger.h"
#include "SplineCurve.h"

std::unique_ptr<DebugDrawManager> DebugDrawManager::instance_ = nullptr;

DebugDrawManager* DebugDrawManager::GetInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = std::make_unique<DebugDrawManager>(ConstructorKey());
	}
	return instance_.get();
}

void DebugDrawManager::Finalize()
{
	instance_.reset();
}

void DebugDrawManager::Initialize() 
{
   RegisterPSO();
   CreateVertexBufferResource();
   CreateTransformationResource();
}

void DebugDrawManager::AddLine(const Vector3& start, const Vector3& end, const Vector4& color) {
    if (vertices_.size() + 2 > maxVertices_) return;
    vertices_.push_back({ Vector4(start.x, start.y, start.z, 1.0f), color });
    vertices_.push_back({ Vector4(end.x, end.y, end.z, 1.0f), color });
}

void DebugDrawManager::AddBox(const Vector3& center, const Vector3& size, const Vector4& color) {
    Vector3 ext = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
    // 8つの頂点を計算
    Vector3 v[8] = {
        { center.x - ext.x, center.y - ext.y, center.z - ext.z },
        { center.x + ext.x, center.y - ext.y, center.z - ext.z },
        { center.x + ext.x, center.y + ext.y, center.z - ext.z },
        { center.x - ext.x, center.y + ext.y, center.z - ext.z },
        { center.x - ext.x, center.y - ext.y, center.z + ext.z },
        { center.x + ext.x, center.y - ext.y, center.z + ext.z },
        { center.x + ext.x, center.y + ext.y, center.z + ext.z },
        { center.x - ext.x, center.y + ext.y, center.z + ext.z }
    };
    // 12本の線分（計24頂点）として登録
    AddLine(v[0], v[1], color); AddLine(v[1], v[2], color); AddLine(v[2], v[3], color); AddLine(v[3], v[0], color); // 手前
    AddLine(v[4], v[5], color); AddLine(v[5], v[6], color); AddLine(v[6], v[7], color); AddLine(v[7], v[4], color); // 奥
    AddLine(v[0], v[4], color); AddLine(v[1], v[5], color); AddLine(v[2], v[6], color); AddLine(v[3], v[7], color); // 繋ぐ線
}

void DebugDrawManager::AddSphereAxis(const Vector3& center, float radius, const Vector4& color, uint32_t segments) {
    // 緯線・経線をLINELISTで繋ぐ（簡易的に3軸の輪っかを描画するだけでもデバッグ用なら十分見やすいです）
    // ここでは一番手軽で視認性の高い「X, Y, Z軸に沿った3本の輪っか」を描画するアプローチを紹介します
    float angleStep = (2.0f * 3.14159265f) / segments;

    for (uint32_t i = 0; i < segments; ++i) {
        float a1 = i * angleStep;
        float a2 = (i + 1) * angleStep;

        // XY平面の輪
        AddLine(center + Vector3(cosf(a1) * radius, sinf(a1) * radius, 0), center + Vector3(cosf(a2) * radius, sinf(a2) * radius, 0), color);
        // XZ平面の輪
        AddLine(center + Vector3(cosf(a1) * radius, 0, sinf(a1) * radius), center + Vector3(cosf(a2) * radius, 0, sinf(a2) * radius), color);
        // YZ平面の輪
        AddLine(center + Vector3(0, cosf(a1) * radius, sinf(a1) * radius), center + Vector3(0, cosf(a2) * radius, sinf(a2) * radius), color);
    }
}

void DebugDrawManager::AddBezier(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2, const uint32_t segments, const Vector4& color)
{
	auto* debugManager = DebugDrawManager::GetInstance();
	Vector3 previousP = controlPoint0;

	for (uint32_t i = 1; i < segments; ++i)
	{
		float t = static_cast<float>(i) / static_cast<float>(segments);
		Vector3 p0p1 = Lerp(controlPoint0, controlPoint1, t);
		Vector3 p1p2 = Lerp(controlPoint1, controlPoint2, t);
		Vector3 currentP = Lerp(p0p1, p1p2, t);

		/*Vector3 nextP0P1 = Lerp(controlPoint0, controlPoint1, t + 1.0f / 32.0f);
		Vector3 nextP1P2 = Lerp(controlPoint1, controlPoint2, t + 1.0f / 32.0f);
		Vector3 nextP = Lerp(nextP0P1, nextP1P2, t + 1.0f / 32.0f);*/
		debugManager->AddLine(previousP, currentP, color);
	}

	Sphere controlSphere[3];
	controlSphere[0] = { controlPoint0, 0.01f };
	controlSphere[1] = { controlPoint1, 0.01f };
	controlSphere[2] = { controlPoint2, 0.01f };

	for (uint32_t i = 0; i < 3; ++i)
	{
		debugManager->AddSphere(controlSphere[i].center, controlSphere[i].radius, { 1.0f, 1.0f, 1.0f, 1.0f });
	}

}

void DebugDrawManager::AddSpline(const std::vector<Vector3>&controlPoints, const Vector4 & color, uint32_t segments)
{
	// 制御点が少なすぎる場合は描画できない
	if (controlPoints.size() < 2 || segments <= 0) return;

	auto* debugManager = DebugDrawManager::GetInstance();

	// 制御点の総数
	size_t numPoints = controlPoints.size();
	size_t numSections = numPoints - 1;

	Vector3 previousP = controlPoints[0];

	for (uint32_t i = 1; i <= segments; ++i)
	{
		// 曲線全体を通した進捗率 (0.0 から 1.0)
		float globalT = static_cast<float>(i) / static_cast<float>(segments);

		// globalT から「現在どの区間にいるか」のインデックスを計算する
		// 例: globalT = 0.5 で 4区間ある場合、 0.5 * 4 = 2.0 -> sectionIndexは2番目
		float sectionProgress = globalT * static_cast<float>(numSections);
		size_t sectionIndex = static_cast<size_t>(sectionProgress);

		// globalT = 1.0 の時など、配列外参照を防ぐためのガード
		if (sectionIndex >= numSections)
		{
			sectionIndex = numSections - 1;
		}

		// その区間内だけのローカルな進捗率 (0.0 から 1.0) に変換
		float localT = sectionProgress - static_cast<float>(sectionIndex);

		// 4つの制御点 (P0, P1, P2, P3) をクランプ処理を交えて取得
		Vector3 p0 = (sectionIndex == 0) ? controlPoints[0] : controlPoints[sectionIndex - 1];
		Vector3 p1 = controlPoints[sectionIndex];
		Vector3 p2 = controlPoints[sectionIndex + 1];
		Vector3 p3 = (sectionIndex + 2 >= numPoints) ? controlPoints[numPoints - 1] : controlPoints[sectionIndex + 2];

		// 現在の補間座標を計算
		Vector3 currentP = CatmullRom(p0, p1, p2, p3, localT);

		// 前回の位置から現在の位置へ線を引く
		debugManager->AddLine(previousP, currentP, color);

		// 次のステップのために現在の座標を保存
		previousP = currentP;
	}

	for (size_t i = 0; i < numPoints; ++i)
	{
		debugManager->AddSphere(controlPoints[i], 0.02f, { 1.0f, 1.0f, 1.0f, 1.0f });
	}
}

void DebugDrawManager::AddLoopSpline(const std::vector<Vector3>& controlPoints, const Vector4& color, uint32_t segments)
{
	// 制御点が少なすぎる、またはセグメント数が0以下の場合は描画できない
	if (controlPoints.size() < 2 || segments <= 0) return;

	auto* debugManager = DebugDrawManager::GetInstance();

	size_t numPoints = controlPoints.size();
	// 変更点: ループするため、区間数は制御点の総数と同じ
	size_t numSections = numPoints;

	// 始点は最初の制御点
	Vector3 previousP = controlPoints[0];

	for (uint32_t i = 1; i <= segments; ++i)
	{
		float globalT = static_cast<float>(i) / static_cast<float>(segments);

		float sectionProgress = globalT * static_cast<float>(numSections);
		size_t sectionIndex = static_cast<size_t>(sectionProgress);

		float localT = 0.0f;
		if (sectionIndex >= numSections)
		{
			sectionIndex = numSections - 1;
			localT = 1.0f;
		} else
		{
			localT = sectionProgress - static_cast<float>(sectionIndex);
		}

		// 変更点: インデックスを循環させる
		size_t idx0 = (sectionIndex + numPoints - 1) % numPoints;
		size_t idx1 = sectionIndex;
		size_t idx2 = (sectionIndex + 1) % numPoints;
		size_t idx3 = (sectionIndex + 2) % numPoints;

		Vector3 p0 = controlPoints[idx0];
		Vector3 p1 = controlPoints[idx1];
		Vector3 p2 = controlPoints[idx2];
		Vector3 p3 = controlPoints[idx3];

		// 現在の補間座標を計算
		Vector3 currentP = CatmullRom(p0, p1, p2, p3, localT);

		// 前回の位置から現在の位置へ線を引く
		debugManager->AddLine(previousP, currentP, color);

		// 次のステップのために現在の座標を保存
		previousP = currentP;
	}

	// 各制御点に球体を描画
	for (size_t i = 0; i < numPoints; ++i)
	{
		debugManager->AddSphere(controlPoints[i], 0.05f, { 1.0f, 1.0f, 1.0f, 1.0f });
	}
}

void DebugDrawManager::AddSphere(const Vector3& center, float radius, const Vector4& color, 
	uint32_t lat, uint32_t lon)
{
	// 最低限の分割数を保証（小さすぎると立体にならないため）
	if (lat < 3) lat = 3;
	if (lon < 3) lon = 3;

	// 仰角（緯度）のステップ数: 北極から南極まで (0 ～ PI)
	uint32_t latSegments = lat;
	// 方位角（経度）のステップ数: 360度周回 (0 ～ 2*PI)
	uint32_t lonSegments = lon * 2; // 経度方向を多めにすると、より均等な格子になります

	const float PI = 3.14159265f;
	float latStep = PI / latSegments;
	float lonStep = (2.0f * PI) / lonSegments;

	// 各格子の点を計算して保持するためのバッファ（動的確保を避けるため、または数式で直接繋いでもOK）
	// ここでは分かりやすさと計算節約のため、1ループ前の点（上の階層の点）を記憶しながら描画します

	// 緯度方向のループ
	for (uint32_t lat = 0; lat <= latSegments; ++lat) {
		float theta = lat * latStep;
		float sinTheta = sinf(theta);
		float cosTheta = cosf(theta);

		// 次の緯度階層のデータ
		float nextTheta = (lat + 1) * latStep;
		float sinNextTheta = sinf(nextTheta);
		float cosNextTheta = cosf(nextTheta);

		// 経度方向のループ
		for (uint32_t lon = 0; lon < lonSegments; ++lon) {
			float phi1 = lon * lonStep;
			float phi2 = (lon + 1) * lonStep;

			// 現在の緯度における、経度方向の2点 (現在の点と、次の点)
			Vector3 pA(sinTheta * cosf(phi1), cosTheta, sinTheta * sinf(phi1));
			Vector3 pB(sinTheta * cosf(phi2), cosTheta, sinTheta * sinf(phi2));

			// 1ステップ下の緯度における、経度方向の点
			Vector3 pC(sinNextTheta * cosf(phi1), cosNextTheta, sinNextTheta * sinf(phi1));

			// スケールと位置の適用
			Vector3 posA = center + pA * radius;
			Vector3 posB = center + pB * radius;
			Vector3 posC = center + pC * radius;

			// 1. 緯線（横の線）を描画
			// 北極点・南極点での無駄な描画をスキップ
			if (lat > 0 && lat < latSegments) {
				AddLine(posA, posB, color);
			}

			// 2. 経線（縦の線）を描画
			if (lat < latSegments) {
				AddLine(posA, posC, color);
			}
		}
	}
}

// --- 実際の描画処理 ---
void DebugDrawManager::DrawAll(const Matrix4x4& viewProjectionMatrix) 
{
    if (vertices_.empty()) 
    {
        return;
    }

    std::memcpy(vertexData_, vertices_.data(), sizeof(DebugVertex) * vertices_.size());
    *transformationData_ = viewProjectionMatrix;

    auto commandList = DirectXBase::GetInstance()->GetCommandList();
    auto& psoSet = PSOManager::GetInstance()->GetPSOData(psoName_, blendMode_, fillMode_);

    DirectXBase::GetInstance()->GetCommandList()->SetPipelineState(psoSet.pipelineState.Get());
    DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootSignature(psoSet.rootSignature.Get());

    DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, transformationResource_->GetGPUVirtualAddress());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->DrawInstanced(static_cast<UINT>(vertices_.size()), 1, 0, 0);

    // 次のフレームのためにクリア
    vertices_.clear();
}

void DebugDrawManager::RegisterPSO()
{
	// PSOの設定
	PSOManager::PSOConfig config{};
	config.vertexShaderPath = L"resources/shaders/DebugDraw/DebugDraw.VS.hlsl";
	config.pixelShaderPath = L"resources/shaders/DebugDraw/DebugDraw.PS.hlsl";
	// Let the rasterizer perform the standard line clipping.
	config.geometryShaderPath = L"";

	// RootSignatureの設定
	config.rootSignatureGenerator = []()
        {
		std::vector<D3D12_ROOT_PARAMETER> rootParameters;
		std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplerDescs;
		
		rootParameters.resize(1);

		// Enum定義 (可読性のため)
		enum {
			kTransform, 
		};

		rootParameters[kTransform].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	// CBVを使う
		rootParameters[kTransform].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	// VertexShaderで使う
		rootParameters[kTransform].Descriptor.ShaderRegister = 0;	// レジスタ番号0とバインド

		// シリアライズ
		static D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
		descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		descriptionRootSignature.pParameters = rootParameters.data();
		descriptionRootSignature.NumParameters = (UINT)rootParameters.size();
		descriptionRootSignature.pStaticSamplers = staticSamplerDescs.data();
		descriptionRootSignature.NumStaticSamplers = (UINT)staticSamplerDescs.size();


		Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
		if (FAILED(hr)) {
			Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
			assert(false);
		}

		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
		hr = DirectXBase::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
		assert(SUCCEEDED(hr));



		return rootSignature;
		};

	config.inputLayoutGenerator = [](){
		return std::vector<D3D12_INPUT_ELEMENT_DESC>
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		};

	// 深度設定
	config.depth.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	config.depthEnable = true;
	config.depthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	config.depthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    config.toporogyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

	PSOManager::GetInstance()->RegisterPSOConfig(psoName_, config);
}

void DebugDrawManager::CreateVertexBufferResource()
{
    // 頂点リソースを作成する。
    vertexResource_ = DirectXBase::GetInstance()->
        CreateBufferResource(sizeof(DebugVertex) * maxVertices_);
    // vertexDataにデータを書き込むためのアドレスを取得
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));   

    // VBVの設定
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(DebugVertex) * maxVertices_;
    vertexBufferView_.StrideInBytes = sizeof(DebugVertex);

    vertices_.reserve(maxVertices_);
}

void DebugDrawManager::CreateTransformationResource()
{
    // 座標変換行列リソースを作成する。Matrix4x4 1つ分のサイズを用意する
    transformationResource_ = DirectXBase::GetInstance()->CreateConstantBufferResource(sizeof(Matrix4x4));
    // TransformationMatrixResourceにデータを書き込むためのアドレスを取得してTransformationMatrixDataに割り当てる
    transformationResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationData_));

    // 座標変換行列データの初期値を書き込む
    *transformationData_ = MakeIdentity4x4();
}
