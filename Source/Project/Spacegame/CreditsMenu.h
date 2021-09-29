#pragma once
#include <GameScene.h>

class CreditsMenu : public GameScene
{
public:
	explicit CreditsMenu();
	virtual ~CreditsMenu() override = default;

	CreditsMenu(const CreditsMenu& other) = delete;
	CreditsMenu(CreditsMenu&& other) noexcept = delete;
	CreditsMenu& operator=(const CreditsMenu& other) = delete;
	CreditsMenu& operator=(CreditsMenu&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;
};

