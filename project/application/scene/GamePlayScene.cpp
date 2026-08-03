#include "GamePlayScene.h"
#include "SceneManager.h"
#include "LightManager.h"
#include "CameraManager.h"
#include "PrimitiveManager.h"
#include "PostEffectManager.h"
#include "Outline.h"
#include "GaussianBlur.h"
#include "RadialBlur.h"
#include "Random.h"
#include "Bloom.h"
#include "Grayscale.h"
#include "BulletManager.h"
#include "SplineCurve.h"
#include "HPGageUI.h"

void GamePlayScene::Initialize()
{
	camera_ = std::make_unique<Camera>();
	camera_->Initialize();

	// Textureを読んで転送する
	textureManager_->LoadTexture("uvChecker.png");
	textureManager_->LoadTexture("monsterBall.png");
	textureManager_->LoadTexture("rostock_laage_airport_4k.dds");

	int textureIndex = 0;
	const char* textureOptions[] = { "Checker", "monsterBall" };

	// シーン初期化始め
	/*camera_->SetRotate({ pi / 3.0f, pi, 0.0f });
	camera_->SetTranslate({ 0.0f, 9.0f, 5.0f });*/

	camera_->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera_->SetTranslate({ 0.0f, 0.0f, -15.0f });

	/*debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	debugCamera_->SetRotate(camera_->GetRotate());
	debugCamera_->SetTranslate(camera_->GetTranslate());*/

	cameraManager_->Initialize();
	cameraManager_->AddCamera("Default", camera_.get());
	/*cameraManager_->AddCamera("Debug", debugCamera_.get());*/
	cameraManager_->SetActiveCamera("Default");

	defaultCameraController_ = std::make_unique<DefaultCameraController>();
	defaultCameraController_->Initialize();

	railCameraController_ = std::make_unique<RailCameraController>();
	railCameraController_->Initialize();

	cameraManager_->AddCameraController("Default", defaultCameraController_.get());
	cameraManager_->AddCameraController("Rail", railCameraController_.get());
	cameraManager_->SetActiveCameraController("Default");

	lightManager_->Initialize();
	lightManager_->SetDirectionalLightColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	lightManager_->SetDirectionalLightDirection({ 0.0f, -1.0f, 0.0f });
	lightManager_->SetDirectionalLightIntensity(1.0f);

	lightManager_->SetNumPointLights(1);
	lightManager_->SetPointLightColor(0, { 1.0f, 0.0f, 0.0f, 1.0f });
	lightManager_->SetPointLightPosition(0, { 0.0f, 1.0f, 0.0f });
	lightManager_->SetPointLightIntensity(0, 1.0f);
	lightManager_->SetPointLightRadius(0, 5.0f);
	lightManager_->SetPointLightDecay(0, 1.0f);

	lightManager_->SetNumSpotLights(1);
	lightManager_->SetSpotLightColor(0, { 0.0f, 1.0f, 0.0f, 1.0f });
	lightManager_->SetSpotLightPosition(0, { 2.0f, 1.25f, 0.0f });
	lightManager_->SetSpotLightIntensity(0, 4.0f);
	lightManager_->SetSpotLightDirection(0, { -1.0f, -1.0f, 0.0f });
	lightManager_->SetSpotLightDistance(0, 7.0f);
	lightManager_->SetSpotLightDecay(0, 1.0f);
	lightManager_->SetSpotLightCosAngle(0, std::cosf(pi / 3.0f));
	lightManager_->SetSpotLightCosFalloff(0, std::cosf(pi / 4.0f));

	for (size_t i = 0; i < 1; i++)
	{
		std::unique_ptr<Sprite> newSprite = std::make_unique<Sprite>();
		newSprite->Initialize("uvChecker.png");
		newSprite->SetAnchorPoint(Vector2{ 0.5f, 0.5f });
		sprites_.push_back(std::move(newSprite));
	}

	sprites_[0]->SetTexture("monsterBall.png");
	sprites_[0]->SetPosition(Vector2{ 100.0f, 100.0f });
	sprites_[0]->AdjustTextureSize();

	modelManager_->LoadModel("", "plane.obj");
	modelManager_->LoadModel("", "axis.obj");
	modelManager_->LoadModel("", "sphere.obj");
	modelManager_->LoadModel("", "multiMaterial.obj");

	for (size_t i = 0; i < 1; i++)
	{
		std::unique_ptr<Object3d> newObject3d = std::make_unique<Object3d>();
		newObject3d->Initialize();
		newObject3d->SetModel("plane.obj");
		object3ds_.push_back(std::move(newObject3d));
	}

	object3ds_[0]->SetModel("sphere.obj");

	/*particleManager_->CreateParticleGroup("circle", "circle.png");
	particleManager_->SetModel("circle", "plane.obj");
	particleManager_->SetIsMoveAccelerationField("circle", true);*/
	particleManager_->CreateParticleGroup("slash", "circle2.png");
	particleManager_->SetModel("slash", "plane.obj");
	particleManager_->SetIsMoveAccelerationField("slash", false);

	PrimitiveManager::GetInstance()->Initialize();
	PrimitiveManager::RingConfig ringConfig;
	ringConfig.segments = 32;
	ringConfig.innerRadius = 0.0f;
	ringConfig.outerRadius = 1.5f;
	ringConfig.innerColor = {0.0f, 0.0f, 1.0f, 1.0f};
	ringConfig.outerColor = {0.0f, 0.5f, 0.0f, 1.0f};
	ringConfig.uvScaleU = 2.0f;
	ringConfig.uvScaleV = 0.1f;
	ringConfig.startAngle = 1.0f / 5.0f * pi;
	ringConfig.endAngle = 9.0f / 5.0f * pi;
	ringConfig.radiusPoints = 
	{
		{ 0.0f, 0.5f, 0.5f },   // 開始：inner=0.2, outer=1.0
		{ 0.5f, 0.5f, 1.0f },   // 中間：inner=0.1, outer=0.6
		{ 1.0f, 0.5f, 0.5f },   // 終了：inner=0.2, outer=0.2
	};
	ringConfig.alphaFade.startFadeRange = 0.15f;
	ringConfig.alphaFade.endFadeRange = 0.15f;
	PrimitiveManager::GetInstance()->CreateRing("ring_primitive", ringConfig);
	
	particleManager_->CreateParticleGroup("ring", "gradationLine.png");
	particleManager_->SetModel("ring", "ring_primitive");
	particleManager_->SetIsMoveAccelerationField("ring", false);
	particleManager_->SetIsBillboard("ring", false);

	PrimitiveManager::CylinderConfig cylinderConfig;
	cylinderConfig.segments = 32;
	cylinderConfig.stacks = 32;
	cylinderConfig.height = 3.0f;
	cylinderConfig.topRadiusX = 1.0f;
	cylinderConfig.topRadiusZ = 1.0f;
	cylinderConfig.bottomRadiusX = 1.0f;
	cylinderConfig.bottomRadiusZ = 1.0f;
	cylinderConfig.radiusPoints =
	{
		{ 0.0f, 1.0f, 1.0f, false },  // ここから折れ線
		{ 0.3f, 2.0f, 2.0f, true  },  // ここからなめらか
		{ 0.7f, 2.0f, 2.0f, false },  // ここから折れ線
		{ 1.0f, 1.0f, 1.0f },
	};
	cylinderConfig.topColor = { 0.0f, 0.0f, 1.0f, 1.0f };
	cylinderConfig.bottomColor = { 0.0f, 0.5f, 0.0f, 1.0f };
	cylinderConfig.uvScaleU = 2.0f;
	cylinderConfig.uvScaleV = 1.0f;
	cylinderConfig.startAngle = 0.0f;
	cylinderConfig.endAngle = 2.0f * pi;
	cylinderConfig.alphaFade.startFadeRange = 0.15f;
	cylinderConfig.alphaFade.endFadeRange = 0.15f;
	PrimitiveManager::GetInstance()->CreateCylinder("cylinder_primitive", cylinderConfig);

	particleManager_->CreateParticleGroup("cylinder", "uvChecker.png");
	particleManager_->SetModel("cylinder", "cylinder_primitive");
	particleManager_->SetIsMoveAccelerationField("cylinder", false);
	particleManager_->SetIsBillboard("cylinder", false);

	AABB  aabb;
	aabb.max = { 1.0f, 1.0f, 1.0f };
	aabb.min = { -1.0f, -1.0f, -1.0f };
	particleManager_->CreateAccelerationField({ 5.0f, 0.0f, 0.0f }, aabb);


	/*for (size_t i = 0; i < 1; i++)
	{
		std::unique_ptr<ParticleEmitter> emitter = std::make_unique<ParticleEmitter>();
		Transform transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
		emitter->Initialize("circle", transform, 3, 0.2f);
		emitters_.push_back(std::move(emitter));
	}*/

	skyBox_ = new SkyBox();
	skyBox_->Initialize();
	skyBox_->SetCamera(camera_.get());

	terrain_ = std::make_unique<Terrain>();
	terrain_->Initialize();

	player_ = std::make_unique<Player>();
	player_->Initialize();
	player_->SetEnvironmentTextureIndex(skyBox_->GetEnvironmentTextureIndex());
	player_->SetParent(cameraManager_->GetActiveCameraController()->GetWorldTransform());

	BulletManager::GetInstance()->Initialize();

	slashEmitter_ = std::make_unique<ParticleEmitter>();
	slashEmitter_->Initialize("slash", 
		{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }, 3, 0.2f);

	ringEmitter_ = std::make_unique<ParticleEmitter>();
	ringEmitter_->Initialize("ring", 
		{ {1.0f, 2.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }, 1, 0.2f);

	cylinderEmitter_ = std::make_unique<ParticleEmitter>();
	cylinderEmitter_->Initialize("cylinder",
		{ {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }, 1, 0.2f);

	auto* postEffectController =
		SceneManager::GetInstance()->GetPostEffectController();
	outlineHandle_ =
		postEffectController->Emit<Outline>("Outline", std::nullopt, nullptr, 1);
	bloomHandle_ =
		postEffectController->Emit<Bloom>("Bloom", std::nullopt, nullptr, 0);
	grayscaleHandle_ =
		postEffectController->Emit<Grayscale>("Grayscale", std::nullopt, nullptr, 2);

	debugManager_->Initialize();

	// シーン初期化終わり

	soundManager_->SoundLoadFile("", "Alarm01.wav");
	soundManager_->SoundLoadFile("", "test.mp3");

	eventBus_ = std::make_unique<EventBus>();
	sceneUI_ = std::make_unique<GamePlaySceneUI>();
	sceneUI_->Initialize(eventBus_.get());

	player_->SetEventBus(eventBus_.get());
	player_->EventDispatch();

	enemyManager_->Initialize(eventBus_.get());
	enemyIDs_.push_back(enemyManager_->AddEnemy());
	for (EnemyID id : enemyIDs_)
	{
		Enemy* enemy = enemyManager_->FindEnemy(id);
		enemy->SetEnvironmentTextureIndex(skyBox_->GetEnvironmentTextureIndex());
		enemy->SetParent(cameraManager_->GetActiveCameraController()->GetWorldTransform());
		enemy->SetEventBus(eventBus_.get());
		enemy->SetHPGageDisplayType(EnemyHPGageDisplayType::ScreenBoss |
			EnemyHPGageDisplayType::OverHead);
		enemy->SetScreenBossPriority(50);
		enemy->EventDispatch();
	}
	
	eventBus_->Dispatch();

	controlSprite_ = std::make_unique<Sprite>();
	controlSprite_->Initialize("Control.png");
	controlSprite_->SetPosition({20.0f, 540.0f});
}

void GamePlayScene::Finalize()
{
	CancelPostEffects();

	enemyManager_->Finalize();
	player_->Finalize();
	
	debugManager_->Finalize();

	//PostEffectManager::GetInstance()->Clear();

	/*for (auto it = emitters_.begin(); it != emitters_.end(); ++it)
	{
		std::unique_ptr<ParticleEmitter> emitter = std::move(*it);
		emitter.reset();

	}*/
	emitters_.clear();

	particleManager_->Reset();

	BulletManager::GetInstance()->Finalize();

	/*for (auto it = object3ds_.begin(); it != object3ds_.end(); ++it)
	{
		std::unique_ptr<Object3d> object3d = std::move(*it);
		object3d.reset();
	}*/
	object3ds_.clear();

	skyBox_->Finalize();
	delete skyBox_;

	/*for (auto it = sprites_.begin(); it != sprites_.end(); ++it)
	{
		std::unique_ptr<Sprite> sprite = std::move(*it);
		sprite.reset();
	}*/
	sprites_.clear();


	camera_->Finalize();
	camera_.reset();

	lightManager_->Finalize();
	cameraManager_->Finalize();
	//PostEffectManager::GetInstance()->Clear();
}

void GamePlayScene::Update(const float& deltaTime)
{
	//if (inputManager_->TriggerKey(DIK_RETURN))
	//{
	//	// シーン切り替え
	//	SceneManager::GetInstance()->SetSceneRequest("TITLE");
	//}

	WorldTransform::AdvanceFrame();

	UpdatePostEffectToggles();

#ifdef _DEBUG

	// デモウィンドウ表示
	ImGui::ShowDemoWindow();

	ImGui::Begin("Setting");
	ImGui::SetWindowSize("Sprite Setting", { 500.0f, 100.0f });
	Vector2 spritePos = sprites_[0]->GetPosition();
	if (ImGui::DragFloat2("pos", &spritePos.x, 1.0f, 0.0f, 0.0f, "%05.1f"))
	{
		sprites_[0]->SetPosition(spritePos);
	}
	ImGui::Checkbox("DrawSprite", &isDrawSprite_);
	ImGui::Checkbox("DrawObject3d", &isDrawObject3d_);
	ImGui::End();


	DrawPostEffectWindow();

	
	ImGui::Begin("LightSetting");

	int32_t enableLighting = object3ds_[0]->GetEnableLighting();
	if (ImGui::Checkbox("EnableLighting", (bool*)&enableLighting))
	{
		object3ds_[0]->SetEnableLighting(enableLighting);
	}

	Vector3 cameraPos = cameraManager_->GetCameraWorldPosition();
	ImGui::InputFloat3("CameraData", &cameraPos.x, "%.3f", ImGuiInputTextFlags_ReadOnly);

	ImGui::End();

	ImGui::Begin("CheckFPS");

	// FPS表示
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

	ImGui::End();

#endif



	/*

	directionalLightData->direction = Vector3{ directionLightDirection.x, directionLightDirection.y, directionLightDirection.z };

	*/


	/*worldMatrixSphere = MakeAffineMatrix(transformSphere.scale, transformSphere.rotate, transformSphere.translate);
	wvpMatrixSphere = Multiply(worldMatrixSphere, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixDataSphere->World = worldMatrixSphere;
	transformationMatrixDataSphere->WVP = wvpMatrixSphere;*/



	lightManager_->Update();

	mousePosition_.x = static_cast<float>(inputManager_->MousePoint().x);
	mousePosition_.y = static_cast<float>(inputManager_->MousePoint().y);

	if (isDebugCamera_)
	{
		/*cameraManager_->SetActiveCamera("Debug");*/
	}
	else
	{
		cameraManager_->SetActiveCamera("Default");
	}

	cameraManager_->Update(deltaTime);




	for (auto it = sprites_.begin(); it != sprites_.end(); ++it)
	{
		Sprite* sprite = it->get();
		sprite->Update();
	}

	for (auto it = object3ds_.begin(); it != object3ds_.end(); ++it)
	{
		Object3d* object3d = it->get();
		object3d->Update();
	}

	for (auto it = emitters_.begin(); it != emitters_.end(); ++it)
	{
		ParticleEmitter* emitter = it->get();
		//emitter->Update(deltaTime);
	}

	particleManager_->Update(deltaTime);

	skyBox_->Update();

	terrain_->Update();

	player_->Update(deltaTime);

	/*enemy_->Update(deltaTime);*/
	enemyManager_->Update(deltaTime);

	BulletManager::GetInstance()->Update(deltaTime);

	if (inputManager_->TriggerKey(DIK_0))
	{
		slashEmitter_->EmitSlash();
	}

	if (inputManager_->TriggerKey(DIK_1))
	{
		ringEmitter_->Emit();
	}

	if (inputManager_->TriggerKey(DIK_2))
	{
		cylinderEmitter_->Emit();
	}


	controlSprite_->Update();

	collisionManager_->Clear();
	if (!player_->GetIsDead())
	{
		collisionManager_->AddCollider(player_->GetCollider());
	}
	
	for (EnemyID id : enemyIDs_)
	{
		if (enemyManager_->FindEnemy(id))
		{
			collisionManager_->AddCollider(enemyManager_->FindEnemy(id)->GetCollider());
			collisionManager_->AddCollider(enemyManager_->FindEnemy(id)->GetAttackCollider());
		}
	}
	
	/*for (auto& enemy : EnemyManager::GetInstance()->GetEnemies())
	{
		collisionManager_->AddCollider(enemy->GetCollider());
	}*/

	for (auto& bullet : BulletManager::GetInstance()->GetBullets())
	{
		collisionManager_->AddCollider(bullet->GetCollider());
	}
	collisionManager_->CheckAllCollisions();

	//// ImGuiの内部コマンドを生成する
	//ImGui::Render();


	eventBus_->Dispatch();
	sceneUI_->Update(deltaTime);

	if (SceneManager::GetInstance()->GetSceneTransition().GetState() == SceneTransition::SceneTransitionState::Idle)
	{
		for (EnemyID id : enemyIDs_)
		{
			if (enemyManager_->GetIsDead(id))
			{
				SceneManager::GetInstance()->SetSceneRequest("RESULT");
			}
		}

		if (player_->GetIsDead())
		{
			SceneManager::GetInstance()->SetSceneRequest("RESULT");
		}
	}

	enemyIDs_.erase(
	std::remove_if(enemyIDs_.begin(), enemyIDs_.end(),
		[this](EnemyID id){ return enemyManager_->FindEnemy(id) == nullptr; }),
	enemyIDs_.end());

	
}

void GamePlayScene::UpdatePostEffectToggles()
{
	auto* controller =
		SceneManager::GetInstance()->GetPostEffectController();

	if (inputManager_->TriggerKey(DIK_F1))
	{
		if (controller->IsActive(outlineHandle_))
		{
			controller->Cancel(outlineHandle_);
			outlineHandle_ = kInvalidPostEffectHandle;
		}
		else
		{
			outlineHandle_ =
				controller->Emit<Outline>("Outline", std::nullopt);
		}
	}

	if (inputManager_->TriggerKey(DIK_F2))
	{
		if (controller->IsActive(gaussianBlurHandle_))
		{
			controller->Cancel(gaussianBlurHandle_);
			gaussianBlurHandle_ = kInvalidPostEffectHandle;
		}
		else
		{
			gaussianBlurHandle_ =
				controller->Emit<GaussianBlur>("GaussianBlur", std::nullopt);
		}
	}

	if (inputManager_->TriggerKey(DIK_F3))
	{
		if (controller->IsActive(radialBlurHandle_))
		{
			controller->Cancel(radialBlurHandle_);
			radialBlurHandle_ = kInvalidPostEffectHandle;
		}
		else
		{
			radialBlurHandle_ =
				controller->Emit<RadialBlur>("RadialBlur", std::nullopt);
		}
	}

	if (inputManager_->TriggerKey(DIK_F4))
	{
		if (controller->IsActive(bloomHandle_))
		{
			controller->Cancel(bloomHandle_);
			bloomHandle_ = kInvalidPostEffectHandle;
		}
		else
		{
			bloomHandle_ =
				controller->Emit<Bloom>("Bloom", std::nullopt);
		}
	}

	if (inputManager_->TriggerKey(DIK_F5))
	{
		if (controller->IsActive(randomHandle_))
		{
			controller->Cancel(randomHandle_);
			randomHandle_ = kInvalidPostEffectHandle;
		}
		else
		{
			randomHandle_ =
				controller->Emit<Random>("Random", std::nullopt);
		}
	}

	if (inputManager_->TriggerKey(DIK_F6))
	{
		if (controller->IsActive(grayscaleHandle_))
		{
			controller->Cancel(grayscaleHandle_);
			grayscaleHandle_ = kInvalidPostEffectHandle;
		}
		else
		{
			grayscaleHandle_ =
				controller->Emit<Grayscale>("Grayscale", std::nullopt);
		}
	}
}

void GamePlayScene::DrawPostEffectWindow()
{
#ifdef _DEBUG
	auto* controller =
		SceneManager::GetInstance()->GetPostEffectController();

	ImGui::Begin("PostEffectWindow");
	ImGui::TextUnformatted(
		"F1 Outline / F2 GaussianBlur / F3 RadialBlur / F4 Bloom");
	ImGui::TextUnformatted(
		"F5 Random / F6 Grayscale");
	ImGui::Separator();

	bool hasActiveEffect = false;

	if (auto* outline = controller->Get<Outline>(outlineHandle_))
	{
		(void)outline;
		hasActiveEffect = true;
		ImGui::PushID("Outline");
		if (ImGui::CollapsingHeader("F1 Outline",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextUnformatted("No adjustable parameters.");
		}
		ImGui::PopID();
	}

	if (auto* blur = controller->Get<GaussianBlur>(gaussianBlurHandle_))
	{
		hasActiveEffect = true;
		ImGui::PushID("GaussianBlur");
		if (ImGui::CollapsingHeader("F2 GaussianBlur",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			float sigma = blur->GetSigma();
			int kernelRadius = blur->GetKernelRadius();
			if (ImGui::SliderFloat("Sigma", &sigma, 0.1f, 20.0f))
			{
				blur->SetSigma(sigma);
			}
			if (ImGui::SliderInt(
				"Kernel Radius", &kernelRadius, 1, 64))
			{
				blur->SetKernelRadius(kernelRadius);
			}
		}
		ImGui::PopID();
	}

	if (auto* blur = controller->Get<RadialBlur>(radialBlurHandle_))
	{
		hasActiveEffect = true;
		ImGui::PushID("RadialBlur");
		if (ImGui::CollapsingHeader("F3 RadialBlur",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			Vector2 center = blur->GetCenter();
			float blurWidth = blur->GetBlurWidth();
			if (ImGui::SliderFloat2(
				"Center", &center.x, 0.0f, 1.0f))
			{
				blur->SetCenter(center);
			}
			if (ImGui::SliderFloat(
				"Blur Width", &blurWidth, 0.0f, 0.05f, "%.4f"))
			{
				blur->SetBlurWidth(blurWidth);
			}
		}
		ImGui::PopID();
	}

	if (auto* bloom = controller->Get<Bloom>(bloomHandle_))
	{
		hasActiveEffect = true;
		ImGui::PushID("Bloom");
		if (ImGui::CollapsingHeader("F4 Bloom",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			float sigma = bloom->GetSigma();
			int kernelRadius = bloom->GetKernelRadius();
			if (ImGui::SliderFloat("Sigma", &sigma, 0.1f, 20.0f))
			{
				bloom->SetSigma(sigma);
			}
			if (ImGui::SliderInt(
				"Kernel Radius", &kernelRadius, 1, 64))
			{
				bloom->SetKernelRadius(kernelRadius);
			}
		}
		ImGui::PopID();
	}

	if (auto* random = controller->Get<Random>(randomHandle_))
	{
		(void)random;
		hasActiveEffect = true;
		ImGui::PushID("Random");
		if (ImGui::CollapsingHeader("F5 Random",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextUnformatted(
				"Time is updated automatically by ChronoManager.");
		}
		ImGui::PopID();
	}

	if (auto* grayscale = controller->Get<Grayscale>(grayscaleHandle_))
	{
		hasActiveEffect = true;
		ImGui::PushID("Grayscale");
		if (ImGui::CollapsingHeader("F6 Grayscale",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			float intensity = grayscale->GetIntensity();
			if (ImGui::SliderFloat(
				"Intensity", &intensity, 0.0f, 1.0f))
			{
				grayscale->SetIntensity(intensity);
			}
		}
		ImGui::PopID();
	}

	if (!hasActiveEffect)
	{
		ImGui::TextDisabled(
			"No active post effects. Press F1-F6 to add one.");
	}

	ImGui::End();
#endif
}

void GamePlayScene::CancelPostEffects()
{
	auto* controller =
		SceneManager::GetInstance()->GetPostEffectController();

	const PostEffectHandle handles[] =
	{
		outlineHandle_,
		gaussianBlurHandle_,
		radialBlurHandle_,
		bloomHandle_,
		randomHandle_,
		grayscaleHandle_,
	};

	for (PostEffectHandle handle : handles)
	{
		if (controller->IsActive(handle))
		{
			controller->Cancel(handle);
		}
	}

	outlineHandle_ = kInvalidPostEffectHandle;
	gaussianBlurHandle_ = kInvalidPostEffectHandle;
	radialBlurHandle_ = kInvalidPostEffectHandle;
	bloomHandle_ = kInvalidPostEffectHandle;
	randomHandle_ = kInvalidPostEffectHandle;
	grayscaleHandle_ = kInvalidPostEffectHandle;
}

void GamePlayScene::FinishFadeIn()
{
	
}

void GamePlayScene::Draw()
{

	skyBox_->Draw();

	if (isDrawObject3d_)
	{
		/*for (size_t i = 0; i < object3ds_.size(); i++)
		{
			object3ds_[i]->Draw();
		}*/

		terrain_->Draw();
		player_->Draw();
		/*enemy_->Draw();*/
		enemyManager_->Draw();
		BulletManager::GetInstance()->Draw();
	}

	
	


	particleManager_->Draw();

	/*for (size_t i = 0; i < particle3ds.size(); i++)
	{
		particle3ds[i]->Draw();
	}*/

	//// SphereのIndexBufferViewを設定
	//commandList->IASetIndexBuffer(&indexBufferView);
	// 
	//// Sphereの描画
	//dxBase->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSphere);	// VBVを設定
	//// SphereのIndexBufferViewを設定
	//dxBase->GetCommandList()->IASetIndexBuffer(&indexBufferViewSphere);
	//// SphereのマテリアルのCBufferの場所を設定
	//dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSphere->GetGPUVirtualAddress());
	//// SphereのTransformationMatrixCBufferの場所を設定
	//dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationResourceSphere->GetGPUVirtualAddress());
	//// SRV用のDescriptorTableの先頭を設定。2はrootParameter[2]である。
	//dxBase->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandles[0]);	// SphereはCheckerを使う
	//// 平行光源用のCBufferをバインド（rootParameter[3] = b1）
	//dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(3, DirectionalLightResource->GetGPUVirtualAddress());
	//// 描画！（DrawCall/ドローコール）。
	//dxBase->GetCommandList()->DrawIndexedInstanced(kSubdivision * kSubdivision * 6, 1, 0, 0, 0);


	if (isDrawSprite_)
	{
		for (size_t i = 0; i < sprites_.size(); i++)
		{
			sprites_[i]->Draw();
		}
	}

	controlSprite_->Draw();

	cameraManager_->DrawDebugUI();

#ifdef _DEBUG
	debugManager_->DrawAll(cameraManager_->GetMainCamera()->GetViewProjectionMatrix());
#endif

	if (inputManager_->TriggerKey(DIK_0))
	{
		soundManager_->SoundPlayWave("Alarm01.wav");
	}
	if (inputManager_->TriggerKey(DIK_1))
	{
		soundManager_->SoundPlayWave("test.mp3");

	}

	sceneUI_->DrawLayer(UILayer::World);
	sceneUI_->DrawLayer(UILayer::Screen);
	sceneUI_->DrawLayer(UILayer::Overlay);
}
