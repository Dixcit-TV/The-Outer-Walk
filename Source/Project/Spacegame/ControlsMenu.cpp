#include "stdafx.h"
#include "ControlsMenu.h"


#include "DebugRenderer.h"
#include "SceneManager.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "SoundManager.h"
#include "SpriteComponent.h"
#include "UI/Button.h"

ControlsMenu::ControlsMenu()
	: GameScene(L"ControlsMenu")
{}

void ControlsMenu::Initialize()
{
	GameObject* pBackground{ new GameObject() };
	SpriteComponent* pBackGroundSprite{ new SpriteComponent(L"Resources/Textures/Game/UI/Controls.png") };
	pBackground->AddComponent(pBackGroundSprite);

	//const auto& window{ OverlordGame::GetGameSettings().Window };
	const std::wstring fontPath{ L"./Resources/Textures/Game/Font/Segoe_Script_Bold_30.fnt" };

	Button* pReturnButton{ new Button(L"RETURN", fontPath, DirectX::XMFLOAT4(DirectX::Colors::White)) };
	pReturnButton->SetHoverColor(DirectX::XMFLOAT4(DirectX::Colors::Orange));
	pReturnButton->GetTransform()->Translate(DirectX::XMFLOAT3(250.f, 550.f, 0.f));
	pReturnButton->RegisterOnClickEvent("ButtonClickedEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuClick.mp3"); });
	pReturnButton->RegisterOnClickEvent("GameStartEvent", []() { SceneManager::GetInstance()->SetActiveGameScene(L"MainMenu"); });
	pReturnButton->RegisterOnHoverEvent("ButtonHoveredEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuHover.mp3", false, 0.5f); });

	AddChild(pBackground);	
	AddChild(pReturnButton);
}

void ControlsMenu::Update()
{
}

void ControlsMenu::Draw()
{
}

