#include "stdafx.h"

#include "SpaceGameScene.h"

#include "GameObject.h"
#include "ContentManager.h"
#include "ModelComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "ColliderComponent.h"

#include "PersistentDataManager.h"
#include "PlanetComponent.h"
#include "PlanetGenerator.h"
#include "PlayerController.h"
#include "PostOcean.h"
#include "SceneManager.h"
#include "SkyBox.h"
#include "SoundManager.h"

SpaceGameScene::SpaceGameScene() :
	GameScene(L"SpaceGameScene")
	, m_pCharacter(nullptr)
	, m_pPlanet(nullptr)
	, m_pPlanetComponent(nullptr)
	, m_pOceanPostProcessing(nullptr)
	, m_UpdatePlanet(false)
{}

SpaceGameScene::~SpaceGameScene()
{
	PlanetGenerator::DestroyInstance();
}

void SpaceGameScene::Initialize() //WELCOME TO THE WALKING SIMULATOR
{
	SoundManager::GetInstance()->CacheSound("Resources/Audio/FX/Collect.mp3");
	
	using namespace DirectX;
	const auto& gameContext{ GetGameContext() };
	
	PlanetGenerator::GetInstance()->Init(gameContext.pDevice);

	SoundManager::GetInstance()->CacheSound("Resources/Audio/FX/Teleport.wav");

	auto* pSkyBox{ new SkyBox(L"Resources/CubeMap/Space.dds") };
	AddChild(pSkyBox);
}

void SpaceGameScene::Update()
{
	using namespace DirectX;
	
	const auto& gc = GetGameContext();

	GameStates state;
	if (!PersistentDataManager::GetInstance()->Get("GameState", state) || state != GameStates::TELEPORT)
	{
		if (gc.pInput->IsKeyboardKeyDown(VK_ESCAPE) && !gc.pInput->IsKeyboardKeyDown(VK_ESCAPE, true))
		{
			PersistentDataManager::GetInstance()->Set("GameState", GameStates::IN_GAME_MENU);
			SceneManager::GetInstance()->SetActiveGameScene(L"InGameMenu");
		}
	}
}

void SpaceGameScene::Draw()
{}

void SpaceGameScene::SceneActivated()
{
	using namespace DirectX;
	const auto& gameContext{ GetGameContext() };

	gameContext.pInput->ForceMouseToCenter(true);
	gameContext.pInput->CursorVisible(false);

	GameStates state;
	if (PersistentDataManager::GetInstance()->Get("GameState", state))
	{
		switch (state)
		{
		case GameStates::GAME_START:
			{
				const float sunDistance{ 300.f };
				XMFLOAT3 sunDir{ -0.577f, -0.577f, 0.577f };
				XMFLOAT3 sunPos;
				XMStoreFloat3(&sunPos, -XMLoadFloat3(&sunDir) * sunDistance);

				gameContext.pShadowMapper->SetLight(sunPos, sunDir);
				gameContext.pShadowMapper->UpdateLightViewSettings(550.f, 0.01f, sunDistance * 2.f);
				gameContext.pShadowMapper->SetShadowIllumination(0.3f);

				LoadPlanet(false);

				m_pCharacter = new PlayerController(XMFLOAT3(0.f, 7.f, -40.f), 3.f, 5.f, 75.f);
				AddChild(m_pCharacter);
				m_pCharacter->SetCurrentPlanet(m_pPlanetComponent);
				m_pCharacter->RegisterTeleportEvent("TeleportCallbackEvent", [this, &gameContext]()
				{
					PersistentDataManager::GetInstance()->Set("GameState", GameStates::PLAYING);
					PersistentDataManager::GetInstance()->Get("CurrentPlanetDesc", m_pPlanetComponent->GetDesc());
					PlanetGenerator::GetInstance()->Generate(gameContext, m_pPlanetComponent, true, true);
					m_pCharacter->SetCurrentPlanet(m_pPlanetComponent);
				});

				PersistentDataManager::GetInstance()->Set("GameState", GameStates::PLAYING);
			}
			break;
		case GameStates::TELEPORT:
			{
				m_pCharacter->Teleport();
				auto* pChannel = SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/Teleport.wav", false, 0.5f);
				pChannel->setPitch(1.2f);
			}
			break;
		case GameStates::BACK_TO_MAIN:
			SceneManager::GetInstance()->SetActiveGameScene(L"MainMenu");
		default: break;
		}
	}
}

void SpaceGameScene::SceneDeactivated()
{
	const auto& gameContext{ GetGameContext() };
	
	gameContext.pInput->ForceMouseToCenter(false);
	gameContext.pInput->CursorVisible(true);
	
	GameStates state;
	if (PersistentDataManager::GetInstance()->Get("GameState", state))
	{
		switch (state)
		{
		case GameStates::BACK_TO_MAIN:
		case GameStates::GAME_END:	
			RemovePostProcessingEffect(m_pOceanPostProcessing);
			RemoveChild(m_pPlanet, true);
			RemoveChild(m_pCharacter, true);

			m_pCharacter = nullptr;
			m_pPlanetComponent = nullptr;
			m_pOceanPostProcessing = nullptr;
			break;
		case GameStates::IN_GAME_MENU:
			PersistentDataManager::GetInstance()->Set("CurrentPlanetDesc", m_pPlanetComponent->GetDesc());
			break;
		default: break;
		}	
	}
}

void SpaceGameScene::LoadPlanet(bool random)
{
	auto* physX = PhysxManager::GetInstance()->GetPhysics();
	const auto& gameContext{ GetGameContext() };
	
	m_pPlanet = new GameObject();
	m_pPlanetComponent = new PlanetComponent(random);
	m_pPlanet->AddComponent(m_pPlanetComponent);

	RigidBodyComponent* pBody{ new RigidBodyComponent() };
	pBody->SetKinematic(true);
	physx::PxTriangleMesh* pTriangleMesh{ ContentManager::Load<physx::PxTriangleMesh>(L"Resources/Meshes/icosahedron.ovpt") };
	std::shared_ptr<physx::PxGeometry> pGeo{ std::make_shared<physx::PxTriangleMeshGeometry>(pTriangleMesh) };
	const auto* pNullPhysxMat = physX->createMaterial(0.f, 0.f, 0.f);
	ColliderComponent* pCollider{ new ColliderComponent(pGeo, *pNullPhysxMat) };
	pBody->SetCollisionGroup(CollisionGroupFlag::Group1);
	m_pPlanet->AddComponent(pBody);
	m_pPlanet->AddComponent(pCollider);
	AddChild(m_pPlanet);
	
	PlanetGenerator::GetInstance()->Generate(gameContext, m_pPlanetComponent, true, true);

	m_pOceanPostProcessing = new PostOcean(m_pPlanetComponent);
	AddPostProcessingEffect(m_pOceanPostProcessing);
}