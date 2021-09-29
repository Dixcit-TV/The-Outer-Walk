#include "stdafx.h"
#include "OrePrefab.h"

#include "ContentManager.h"
#include "TransformComponent.h"
#include "ModelComponent.h"
#include "ColliderComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "TextRenderer.h"
#include "SpriteFont.h"
#include "OverlordGame.h"
#include "../../Materials/Shadow/DiffuseMaterial_Shadow.h"

OrePrefab::OrePrefab(const DirectX::XMFLOAT3& spawnPosition, const DirectX::XMFLOAT4& spawnRotation)
	: GameObject()
	, m_SpawnRotation(spawnRotation)
	, m_SpawnPosition(spawnPosition)
	, m_PickingTextSize()
	, m_PickingDisplay(L"Press F to pick-up Ore")
	, m_pFont(nullptr)
	, m_Scale(0.5f)
{}

void OrePrefab::Initialize(const GameContext & gameContext)
{
	ModelComponent* pModel = new ModelComponent(L"Resources/Meshes/Game/Ore/BlueOre.ovm");
	if (gameContext.pMaterialManager->GetMaterial(41) == nullptr)
	{
		DiffuseMaterial_Shadow* pDiffuseMat{ new DiffuseMaterial_Shadow() };
		pDiffuseMat->SetDiffuseTexture(L"Resources/Textures/Game/Ore/BlueOre_diffuse.png");
		gameContext.pMaterialManager->AddMaterial(pDiffuseMat, 41);
	}
	pModel->SetMaterial(41);
	AddComponent(pModel);

	auto* physX = PhysxManager::GetInstance()->GetPhysics();
	RigidBodyComponent* pBody{ new RigidBodyComponent() };
	pBody->SetKinematic(true);
	physx::PxConvexMesh* pConvexMesh{ ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/Game/Ore/BlueOre.ovpc") };
	std::shared_ptr<physx::PxGeometry> pGeo{ std::make_shared<physx::PxConvexMeshGeometry>(pConvexMesh, physx::PxMeshScale(m_Scale)) };
	const auto* pNullPhysxMat = physX->createMaterial(0.f, 0.f, 0.f);
	ColliderComponent* pCollider{ new ColliderComponent(pGeo, *pNullPhysxMat) };
	pBody->SetCollisionGroup(CollisionGroupFlag::Group2);
	AddComponent(pBody);
	AddComponent(pCollider);

	m_pFont = ContentManager::Load<SpriteFont>(L"./Resources/Textures/Game/Font/Segoe_Script_Bold_30.fnt");
	m_PickingTextSize = m_pFont->GetTextSize(m_PickingDisplay);
}

void OrePrefab::PostInitialize(const GameContext&)
{
	GetTransform()->Scale(m_Scale, m_Scale, m_Scale);
	GetTransform()->Rotate(XMLoadFloat4(&m_SpawnRotation));
	GetTransform()->Translate(m_SpawnPosition);
}

void OrePrefab::DrawPickingDisplay() const
{
	if (m_pFont)
	{
		const DirectX::XMFLOAT2 position(
			(OverlordGame::GetGameSettings().Window.Width - m_PickingTextSize.x) * 0.5f
			, OverlordGame::GetGameSettings().Window.Height * 0.8f - m_PickingTextSize.y);
		
		TextRenderer::GetInstance()->DrawText(m_pFont, m_PickingDisplay.c_str(), position, DirectX::XMFLOAT4(DirectX::Colors::White));
	}
}