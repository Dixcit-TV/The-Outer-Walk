#pragma once
#include <GameScene.h>

#include "PlanetGenerator.h"
class PlayerController;
class SpriteFont;
class PlanetComponent;
class PlanetMaterial;

class SpaceGameScene final : public GameScene
{
public:
	explicit SpaceGameScene();
	~SpaceGameScene() override;

	SpaceGameScene(const SpaceGameScene& other) = delete;
	SpaceGameScene(SpaceGameScene&& other) noexcept = delete;
	SpaceGameScene& operator=(const SpaceGameScene& other) = delete;
	SpaceGameScene& operator=(SpaceGameScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;

	void SceneActivated() override;
	void SceneDeactivated() override;

private:
	PlayerController* m_pCharacter;
	GameObject* m_pPlanet;
	PlanetComponent* m_pPlanetComponent;
	PostProcessingMaterial* m_pOceanPostProcessing;

	bool m_UpdatePlanet;

	void LoadPlanet(bool random = true);
};

