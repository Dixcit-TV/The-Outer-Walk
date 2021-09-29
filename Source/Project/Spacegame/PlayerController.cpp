#include "stdafx.h"
#include "PlayerController.h"

#include "CameraComponent.h"
#include "FixedCamera.h"
#include "PhysxManager.h"
#include "ControllerComponent.h"
#include "DebugRenderer.h"
#include "GameScene.h"
#include "ModelAnimator.h"
#include "OrePrefab.h"
#include "OverlordGame.h"
#include "PlanetComponent.h"
#include "SoundManager.h"
#include "TransformComponent.h"
#include "Utils.h"
#include "../../Materials/Shadow/SkinnedDiffuseMaterial_Shadow.h"

PlayerController::PlayerController(const DirectX::XMFLOAT3& cameraOffset, float radius, float height, float moveSpeed) :
	m_CameraOffset(cameraOffset),
	m_Velocity(0, 0, 0),
	m_Radius(radius),
	m_Height(height),
	m_MoveSpeed(moveSpeed),
	m_RotationSpeed(50.f),
	m_TotalPitch(0.f),
	m_TotalYaw(0.f),
	//Running
	m_MaxRunVelocity(100.0f),
	m_TerminalVelocity(50),
	m_Gravity(0),
	m_RunAccelerationTime(0.3f),
	m_JumpAccelerationTime(0.2f),
	m_RunAcceleration(m_MaxRunVelocity / m_RunAccelerationTime),
	m_JumpAcceleration(0),
	m_RunVelocity(0),
	m_JumpVelocity(0),
	m_pCamera(nullptr),
	m_pController(nullptr),
	m_pMesh(nullptr),
	m_pPlanet(nullptr),
	m_pTeleportGo(nullptr),
	m_ParticleSystems(),
	m_CurrentAnim(L"Idle"),
	m_TeleportDuration(5.f),
	m_TeleportTimer(0.f),
	m_IsTeleporting(false)
{}

void PlayerController::Initialize(const GameContext& gameContext)
{
	//TODO: Create controller
	auto* pPhysx{ PhysxManager::GetInstance()->GetPhysics() };
	auto* pMaterial{ pPhysx->createMaterial(8.f, 6.f, 0.f) };
	m_pController = new ControllerComponent(pMaterial, m_Radius, m_Height, L"Character");
	AddComponent(m_pController);

	//TODO: Add a fixed camera as child
	auto* pCamera{ new FixedCamera() };
	m_pCamera = new CameraComponent();
	pCamera->AddComponent(m_pCamera);
	AddChild(pCamera);

	GameObject* pMeshGO{ new GameObject() };
	m_pMesh = new ModelComponent(L"Resources/Meshes/Game/Spaceman.ovm");

	const UINT characterMaterialId{ 72 };
	if (gameContext.pMaterialManager->GetMaterial(characterMaterialId) == nullptr)
	{
		SkinnedDiffuseMaterial_Shadow* pDiffuseMat{ new SkinnedDiffuseMaterial_Shadow() };
		pDiffuseMat->SetDiffuseTexture(L"Resources/Textures/Game/Spaceman/default_Base_Color.png");
		gameContext.pMaterialManager->AddMaterial(pDiffuseMat, characterMaterialId);
	}
	
	m_pMesh->SetMaterial(characterMaterialId);
	pMeshGO->AddComponent(m_pMesh);
	AddChild(pMeshGO);

	ParticleEmitterComponent* pStepParticles{ new ParticleEmitterComponent(L"Resources/Textures/Game/Particles/StepSmoke.png", false) };
	pStepParticles->SetVelocity(DirectX::XMFLOAT3(0, 2.0f, 0));
	pStepParticles->SetMinSize(3.0f);
	pStepParticles->SetMaxSize(5.0f);
	pStepParticles->SetMinEnergy(0.5f);
	pStepParticles->SetMaxEnergy(1.0f);
	pStepParticles->SetMinSizeGrow(0.2f);
	pStepParticles->SetMaxSizeGrow(0.5f);
	pStepParticles->SetMinEmitterRange(0.2f);
	pStepParticles->SetMaxEmitterRange(0.5f);
	pStepParticles->SetColor(DirectX::XMFLOAT4(1.f, 1.f, 1.f, 0.6f));

	GameObject* pStepParticleSystem{ new GameObject() };
	pStepParticleSystem->AddComponent(pStepParticles);
	m_ParticleSystems["StepParticles"] = pStepParticles;
	AddChild(pStepParticleSystem);


	m_pTeleportGo = new GameObject();
	auto* pSubGo{ new GameObject() };
	auto* pParticule{ new ParticleEmitterComponent(L"Resources/Textures/Game/Particles/circle_01.png", false, 5,  false) };
	pParticule->SetVelocity(DirectX::XMFLOAT3(0, 0, 0));
	pParticule->SetMinSize(3.0f);
	pParticule->SetMaxSize(3.0f);
	pParticule->SetMinEnergy(3.f);
	pParticule->SetMaxEnergy(5.f);
	pParticule->SetMinSizeGrow(4.f);
	pParticule->SetMaxSizeGrow(4.f);
	pParticule->SetMinEmitterRange(0.f);
	pParticule->SetMaxEmitterRange(0.f);
	pParticule->SetColor(DirectX::XMFLOAT4(0.211f, 0.780f, 0.545f, 0.6f));
	pParticule->SetAngularVelocity(DirectX::XM_PIDIV4 * 0.5f);

	m_ParticleSystems["TeleportParticles1"] = pParticule;
	pSubGo->AddComponent(pParticule);
	pSubGo->GetTransform()->Rotate(90.f, 0.f, 0.f);

	pParticule = new ParticleEmitterComponent(L"Resources/Textures/Game/Particles/trace_02.png", false, 40, true);
	pParticule->SetVelocity(DirectX::XMFLOAT3(0, 20.f, 0));
	pParticule->SetMinSize(3.0f);
	pParticule->SetMaxSize(5.0f);
	pParticule->SetMinEnergy(1.f);
	pParticule->SetMaxEnergy(2.f);
	pParticule->SetMinSizeGrow(0.5f);
	pParticule->SetMaxSizeGrow(0.2f);
	pParticule->SetMinEmitterRange(4.5f);
	pParticule->SetMaxEmitterRange(5.f);
	pParticule->SetColor(DirectX::XMFLOAT4(0.211f, 0.780f, 0.545f, 0.6f));
	pParticule->SetAngularVelocity(DirectX::XM_PIDIV4 * 0.5f);
	
	m_ParticleSystems["TeleportParticles2"] = pParticule;
	m_pTeleportGo->AddComponent(pParticule);
	m_pTeleportGo->AddChild(pSubGo);
	AddChild(m_pTeleportGo);

	//TODO: Register all Input Actions
	gameContext.pInput->AddInputAction(InputAction(static_cast<int>(Inputs::LEFT), InputTriggerState::Down, 'A', -1, XINPUT_GAMEPAD_DPAD_LEFT));
	gameContext.pInput->AddInputAction(InputAction(static_cast<int>(Inputs::RIGHT), InputTriggerState::Down, 'D', -1, XINPUT_GAMEPAD_DPAD_RIGHT));
	gameContext.pInput->AddInputAction(InputAction(static_cast<int>(Inputs::FORWARD), InputTriggerState::Down, 'W', -1, XINPUT_GAMEPAD_DPAD_UP));
	gameContext.pInput->AddInputAction(InputAction(static_cast<int>(Inputs::BACKWARD), InputTriggerState::Down, 'S', -1, XINPUT_GAMEPAD_DPAD_DOWN));
	gameContext.pInput->AddInputAction(InputAction(static_cast<int>(Inputs::JUMP), InputTriggerState::Down, VK_SPACE, -1, XINPUT_GAMEPAD_A));
}

void PlayerController::PostInitialize(const GameContext&)
{
	//TODO: Set the camera as active
	// We need to do this in the PostInitialize because child game objects only get initialized after the Initialize of the current object finishes
	m_pCamera->SetActive();
	m_pMesh->GetTransform()->Scale(0.05f, 0.05f, 0.05f);
	m_pMesh->GetTransform()->Rotate(0.f, 180.f, 0.f);

	const DirectX::XMFLOAT3 groundPos{ 0.f, -m_Radius - m_Height * 0.5f, 0.f };
	m_pMesh->GetTransform()->Translate(groundPos);
	m_pTeleportGo->GetTransform()->Translate(groundPos);
	m_pCamera->GetTransform()->Translate(m_CameraOffset);

	m_ParticleSystems["StepParticles"]->GetTransform()->Translate(0.f, -(m_Height + 2.f * m_Radius) * 0.5f, 0.f);

	SelectAnimation(DirectX::XMFLOAT2{0.f, 0.f});
	m_pMesh->GetAnimator()->Play();
}

void PlayerController::Update(const GameContext& gameContext)
{
	UpdateGravity();
	if (m_pCamera->IsActive())
	{
		if (m_IsTeleporting)
		{
			m_TeleportTimer += gameContext.pGameTime->GetElapsed();
			if (m_TeleportTimer >= m_TeleportDuration)
			{
				m_TeleportTimer = 0.f;
				m_IsTeleporting = false;
				m_ParticleSystems["TeleportParticles1"]->Stop(true);
				m_ParticleSystems["TeleportParticles2"]->Stop(true);
				OnTeleporOverEvent.Invoke();
			}
		}
		else
		{
			ProcessMovements(gameContext);
			ProcessInteractions(gameContext);
		}
		
		//Replace character based on the transformation of the plent them are standing on (rotation only)
		DirectX::XMFLOAT4 frameRot{ m_pPlanet->GetFrameRotation(gameContext.pGameTime->GetElapsed()) };
		const DirectX::XMVECTOR planetRotation{XMLoadFloat4(&frameRot) };
		const DirectX::XMVECTOR newRotation{DirectX::XMQuaternionMultiply(XMLoadFloat4(&GetTransform()->GetRotation()), planetRotation) };
		const DirectX::XMVECTOR newWorldPos{ XMVector3Transform(XMLoadFloat3(&GetTransform()->GetPosition()), DirectX::XMMatrixRotationQuaternion(planetRotation)) };
		GetTransform()->Rotate(newRotation);
		GetTransform()->Translate(newWorldPos);
	}
}

void PlayerController::ProcessMovements(const GameContext& gameContext)
{
	using namespace DirectX;

	auto* pInputManager{ gameContext.pInput };
	//Get Input Translation
	auto move = XMFLOAT2(0, 0);
	move.y = pInputManager->IsActionTriggered(static_cast<int>(Inputs::FORWARD)) ? 1.0f : 0.0f;
	if (move.y == 0) move.y = -(pInputManager->IsActionTriggered(static_cast<int>(Inputs::BACKWARD)) ? 1.0f : 0.0f);
	if (move.y == 0) move.y = InputManager::GetThumbstickPosition().y;

	move.x = pInputManager->IsActionTriggered(static_cast<int>(Inputs::RIGHT)) ? 1.0f : 0.0f;
	if (move.x == 0) move.x = -(pInputManager->IsActionTriggered(static_cast<int>(Inputs::LEFT)) ? 1.0f : 0.0f);
	if (move.x == 0) move.x = InputManager::GetThumbstickPosition().x;

	XMStoreFloat2(&move, XMVector2Normalize(XMLoadFloat2(&move)));

	const float elapsedTime{ gameContext.pGameTime->GetElapsed() };
	XMFLOAT3 f3Up{ m_pController->GetUpVector() };

	//Calculate character local matrix
	const auto viewForward = XMLoadFloat3(&GetTransform()->GetForward());
	XMVECTOR xmUp{ XMLoadFloat3(&f3Up) };
	XMVECTOR right{ XMVector3Cross(xmUp, viewForward) };

	XMFLOAT3 f3Forward, f3Right;
	XMStoreFloat3(&f3Forward, viewForward);
	XMStoreFloat3(&f3Right, right);

	physx::PxMat33 lookAtMatrix{ ToPxVec3(f3Right), ToPxVec3(f3Up), ToPxVec3(f3Forward) };

	m_Velocity = XMFLOAT3{};
	if (move.x != 0 || move.y != 0)
	{
		m_RunVelocity += m_RunAcceleration * elapsedTime;
		Clamp(m_RunVelocity, m_MaxRunVelocity, 0.f);

		const auto direction{ viewForward * move.y + right * move.x };
		XMStoreFloat3(&m_Velocity, direction * m_MoveSpeed);

		//Rotate the mesh towards the moving direction, based on input
		float meshYaw;
		float cross{};
		XMVECTOR movementVec{ XMLoadFloat2(&move) };
		XMVECTOR refVec{ XMVectorSet(0.f, 1.f, 0.f, 0.f) };
		XMStoreFloat(&cross, XMVector2Cross(movementVec, refVec));
		XMStoreFloat(&meshYaw, XMVector2AngleBetweenVectors(movementVec, refVec));
		physx::PxQuat meshRot{ (cross > 0 ? meshYaw : -meshYaw) + XM_PI, physx::PxVec3(0.f, 1.f, 0.f) };
		m_pMesh->GetTransform()->Rotate(XMVectorSet(meshRot.x, meshRot.y, meshRot.z, meshRot.w));
	}

	const float jumpStartSpeed = 60.f;
	const bool isGrounded{ m_pController->GetCollisionFlags() & physx::PxControllerCollisionFlag::eCOLLISION_DOWN };
	if (!isGrounded)
	{
		m_JumpVelocity -= m_JumpAcceleration * elapsedTime;
		Clamp(m_JumpVelocity, jumpStartSpeed, -m_TerminalVelocity);

		auto velocity = XMLoadFloat3(&m_Velocity);
		XMStoreFloat3(&m_Velocity, velocity * 0.33f);
	}
	else if (pInputManager->IsActionTriggered(static_cast<int>(Inputs::JUMP)))
	{
		m_JumpVelocity = jumpStartSpeed;
	}
	else
		m_JumpVelocity = 0.f;

	XMFLOAT3 jumpVelVec{};
	XMStoreFloat3(&jumpVelVec, xmUp * m_JumpVelocity);

	//Get input Rotation
	const auto mouseMove = InputManager::GetMouseMovement();
	auto look = XMFLOAT2(static_cast<float>(mouseMove.x), static_cast<float>(mouseMove.y));
	if (look.x == 0 && look.y == 0)
	{
		look = InputManager::GetThumbstickPosition(false);
		look.y *= -1.f;
	}

	//CALCULATE TRANSFORMS
	auto displacement = XMLoadFloat3(&m_Velocity) + XMLoadFloat3(&jumpVelVec);
	displacement *= elapsedTime;

	XMFLOAT3 f3Dist;
	XMStoreFloat3(&f3Dist, displacement);

	float yaw = look.x * m_RotationSpeed * elapsedTime;
	m_TotalPitch += look.y * m_RotationSpeed * elapsedTime;

	const float pitchLimit{ 80.f };
	Clamp(m_TotalPitch, pitchLimit, -pitchLimit);

	physx::PxQuat pitchQuat{ XMConvertToRadians(m_TotalPitch), physx::PxVec3(1.f, 0.f, 0.f) };
	physx::PxQuat yawQuat{ XMConvertToRadians(yaw), physx::PxVec3(0.f, 1.f, 0.f) };
	physx::PxQuat rotation{ physx::PxQuat(lookAtMatrix) * yawQuat };

	GetTransform()->Rotate(XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w));

	auto cameraPos{ pitchQuat.rotate(ToPxVec3(m_CameraOffset)) };
	m_pCamera->GetTransform()->Rotate(XMVectorSet(pitchQuat.x, pitchQuat.y, pitchQuat.z, pitchQuat.w));
	m_pCamera->GetTransform()->Translate(ToXMFLOAT3(cameraPos));

	m_pController->Move(f3Dist);

	SelectAnimation(move);
	auto foot = m_pController->GetPosition();

	GetTransform()->Translate(foot);
}

void PlayerController::ProcessInteractions(const GameContext&) const
{
	GameObject* pPickedObject{ m_pCamera->SpherePick(10.f, 50.f,
		{ static_cast<float>(OverlordGame::GetGameSettings().Window.Width) * 0.5f
		, static_cast<float>(OverlordGame::GetGameSettings().Window.Height) * 0.5f }
		, CollisionGroupFlag::Group0 | CollisionGroupFlag::Group1) };
	
	if (pPickedObject)
	{
		OrePrefab* pOre{ dynamic_cast<OrePrefab*>(pPickedObject) };
		if (pOre)
		{
			pOre->DrawPickingDisplay();
			if (InputManager::IsKeyboardKeyDown('F') && !InputManager::IsKeyboardKeyDown('F', true))
			{
				pPickedObject->GetParent()->RemoveChild(pPickedObject);
				SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/Collect.mp3");
			}
		}
	}
}

void PlayerController::UpdateGravity()
{
	using namespace DirectX;

	XMFLOAT3 upVector;
	XMStoreFloat3(&upVector, XMVector3Normalize(XMLoadFloat3(&GetTransform()->GetPosition()) - XMLoadFloat3(&m_pPlanet->GetTransform()->GetPosition())));
	
	m_Gravity = m_pPlanet->GetGravity();
	m_JumpAcceleration = -m_Gravity / m_JumpAccelerationTime;
	m_pController->UpdateUpVector(upVector);
}

void PlayerController::SelectAnimation(const DirectX::XMFLOAT2& xzDisplacement)
{
	DirectX::XMVECTOR dist{ XMLoadFloat2(&xzDisplacement) };
	float vel;
	DirectX::XMStoreFloat(&vel, DirectX::XMVector2LengthSq(dist));

	std::wstring newAnim{ };
	const DirectX::XMFLOAT3& position{ m_pController->GetFootPosition() };

	physx::PxQueryFilterData filterData;
	filterData.data.word0 = ~static_cast<physx::PxU32>(CollisionGroupFlag::Group0);
	filterData.flags |= physx::PxQueryFlag::eANY_HIT;

	const physx::PxSphereGeometry overlapShape{ 3.f };
	PhysxProxy* pPhysxProx{ GetScene()->GetPhysxProxy() };
	physx::PxOverlapBuffer hit;
	if (!pPhysxProx->Overlap(overlapShape, physx::PxTransform(ToPxVec3(position)), hit, filterData))
	{
		newAnim = L"FallIdle";
	}
	else {
		if (vel > 0.01f)
		{
			newAnim = L"Running";
			m_ParticleSystems["StepParticles"]->Play();
		}
		else	 
		{
			newAnim = L"Idle";
		}
	}

	if (m_CurrentAnim != newAnim)
	{
		m_CurrentAnim = newAnim;
		m_pMesh->GetAnimator()->SetAnimation(newAnim);
	}

	if (m_CurrentAnim != L"Running" && m_ParticleSystems["StepParticles"]->IsPlaying())
	{
		m_ParticleSystems["StepParticles"]->Stop();
	}
}

void PlayerController::SetCurrentPlanet(PlanetComponent* pPlanet)
{
	using namespace DirectX;

	m_pPlanet = pPlanet;
	const XMVECTOR randRotation{ XMQuaternionRotationRollPitchYaw(randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI)) };
	const float height{ static_cast<float>(m_pPlanet->GetDesc().Scaling) + m_pPlanet->GetMaxHeight() + 1.f };
	XMFLOAT3 translation;
	XMStoreFloat3(&translation, XMVector3Rotate(XMVectorSet(0.f, 1.f, 0.f, 0.f), randRotation) * height);
	Translate(translation);
	UpdateGravity();
}

void PlayerController::Translate(const DirectX::XMFLOAT3& position) const
{
	GetTransform()->Translate(position);
	m_pController->Translate(position);
}

void PlayerController::Teleport()
{
	if (m_IsTeleporting)
		return;
	
	m_IsTeleporting = true;
	
	m_ParticleSystems["TeleportParticles1"]->Play();
	m_ParticleSystems["TeleportParticles2"]->Play();
}