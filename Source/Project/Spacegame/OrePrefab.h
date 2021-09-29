#pragma once
#include <GameObject.h>

class SpriteFont;

class OrePrefab : public GameObject
{
public:
	explicit OrePrefab(const DirectX::XMFLOAT3& spawnPosition, const DirectX::XMFLOAT4& spawnRotation = {0.f, 0.f, 0.f, 1.f});
	virtual ~OrePrefab() = default;

	OrePrefab(const OrePrefab& other) = delete;
	OrePrefab(OrePrefab&& other) noexcept = delete;
	OrePrefab& operator=(const OrePrefab& other) = delete;
	OrePrefab& operator=(OrePrefab&& other) noexcept = delete;

	void Initialize(const GameContext& gameContext) override;
	void PostInitialize(const GameContext& gameContext) override;
	void DrawPickingDisplay() const;

private:
	DirectX::XMFLOAT4 m_SpawnRotation;
	DirectX::XMFLOAT3 m_SpawnPosition;
	DirectX::XMFLOAT2 m_PickingTextSize;;
	std::wstring m_PickingDisplay;
	SpriteFont* m_pFont;
	float m_Scale;
};

