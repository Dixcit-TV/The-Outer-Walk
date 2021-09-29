#pragma once
#include <GameObject.h>
#include <ModelComponent.h>
#include <ParticleEmitterComponent.h>

#include "Delegate.h"

class PlanetComponent;
class ModelComponent;
class CameraComponent;
class ControllerComponent;
struct GameContext;

class PlayerController : public GameObject
{
public:
	explicit PlayerController(const DirectX::XMFLOAT3& cameraOffset, float radius = 2, float height = 5, float moveSpeed = 100);
	virtual ~PlayerController() = default;

	PlayerController(const PlayerController& other) = delete;
	PlayerController(PlayerController&& other) noexcept = delete;
	PlayerController& operator=(const PlayerController& other) = delete;
	PlayerController& operator=(PlayerController&& other) noexcept = delete;
	
	void Initialize(const GameContext& gameContext) override;
	void PostInitialize(const GameContext& gameContext) override;
	void Update(const GameContext& gameContext) override;
	void SetCurrentPlanet(PlanetComponent* pPlanet);

	void Teleport();

	void RegisterTeleportEvent(const std::string& evtName, std::function<void()> callback) { OnTeleporOverEvent.Register(evtName, callback); }

private:
	enum class Inputs
	{
		LEFT = 0,
		RIGHT,
		FORWARD,
		BACKWARD,
		JUMP
	};

	DirectX::XMFLOAT3 m_CameraOffset;
	DirectX::XMFLOAT3 m_Velocity;

	float m_Radius, m_Height;
	float m_MoveSpeed, m_RotationSpeed;
	float m_TotalPitch, m_TotalYaw;

	//Running
	float m_MaxRunVelocity,
		m_TerminalVelocity,
		m_Gravity,
		m_RunAccelerationTime,
		m_JumpAccelerationTime,
		m_JumpAcceleration,
		m_RunAcceleration,
		m_RunVelocity,
		m_JumpVelocity;

	CameraComponent* m_pCamera;
	ControllerComponent* m_pController;
	ModelComponent* m_pMesh;
	PlanetComponent* m_pPlanet;
	GameObject* m_pTeleportGo;
	std::map<std::string, ParticleEmitterComponent*> m_ParticleSystems;

	std::wstring m_CurrentAnim;

	Event<> OnTeleporOverEvent;
	const float m_TeleportDuration;
	float m_TeleportTimer;
	bool m_IsTeleporting;

	void UpdateGravity();
	void ProcessMovements(const GameContext& gameContext);
	void ProcessInteractions(const GameContext& gameContext) const;
	void SelectAnimation(const DirectX::XMFLOAT2& xzDisplacement);
	void Translate(const DirectX::XMFLOAT3& position) const;
};