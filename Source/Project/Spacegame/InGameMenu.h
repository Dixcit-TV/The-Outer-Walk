#pragma once
#include <GameScene.h>

class PlanetComponent;
struct NoiseParameter;

class InGameMenu : public GameScene
{
public:
	explicit InGameMenu();
	virtual ~InGameMenu() override;

	InGameMenu(const InGameMenu& other) = delete;
	InGameMenu(InGameMenu&& other) noexcept = delete;
	InGameMenu& operator=(const InGameMenu& other) = delete;
	InGameMenu& operator=(InGameMenu&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void SceneActivated() override;
	void SceneDeactivated() override;

private:

	CameraComponent* m_pCamera;
	GameObject* m_pPlanet;
	PlanetComponent* m_pPlanetComponent;
	PostProcessingMaterial* m_pOceanPostProcessing;
	bool m_UpdatePlanet;

	void SetNewCurrentPlanet() const;
	void DrawNoiseParameterUI(NoiseParameter& np, int idx);
};

