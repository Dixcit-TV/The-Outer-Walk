#pragma once
#include <GameScene.h>
class ControlsMenu : public GameScene
{
public:
	explicit ControlsMenu();
	virtual ~ControlsMenu() override = default;

	ControlsMenu(const ControlsMenu& other) = delete;
	ControlsMenu(ControlsMenu&& other) noexcept = delete;
	ControlsMenu& operator=(const ControlsMenu& other) = delete;
	ControlsMenu& operator=(ControlsMenu&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;
};

