#include "stdafx.h"
#include "MainMenu.h"

#include "GameObject.h"
#include "SceneManager.h"
#include "TransformComponent.h"
#include "UI/Button.h"
#include <SpriteComponent.h>



#include "DebugRenderer.h"
#include "HelperStruct.h"
#include "PersistentDataManager.h"
#include "SoundManager.h"

MainMenu::MainMenu()
	: GameScene(L"MainMenu")
{}

void MainMenu::Initialize()
{
	DebugRenderer::ToggleDebugRenderer();
	
	auto* pFmodSystem{ SoundManager::GetInstance()->GetSystem() };
	FMOD::Sound* pMainThemSound;
	pFmodSystem->createStream("Resources/Audio/Music/MainTheme.mp3", FMOD_LOOP_NORMAL | FMOD_2D, nullptr, &pMainThemSound);

	FMOD::Channel* pChannel;
	pFmodSystem->playSound(pMainThemSound, nullptr, false, &pChannel);
	pChannel->setLoopCount(-1);

	SoundManager::GetInstance()->CacheSound("Resources/Audio/FX/MenuClick.mp3");
	SoundManager::GetInstance()->CacheSound("Resources/Audio/FX/MenuHover.mp3");
	
	GameObject* pBackground{ new GameObject() };
	SpriteComponent* pBackGroundSprite{ new SpriteComponent(L"Resources/Textures/Game/UI/Background.png") };
	pBackground->AddComponent(pBackGroundSprite);
	
	const std::wstring fontPath{ L"./Resources/Textures/Game/Font/Segoe_Script_Bold_30.fnt" };

	Button* pPlayButton{ new Button(L"START", fontPath, DirectX::XMFLOAT4(DirectX::Colors::White)) };
	pPlayButton->SetHoverColor(DirectX::XMFLOAT4(DirectX::Colors::Orange));
	pPlayButton->GetTransform()->Translate(DirectX::XMFLOAT3(250.f, 250.f, 0.f));
	pPlayButton->RegisterOnClickEvent("ButtonClickedEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuClick.mp3"); });
	pPlayButton->RegisterOnClickEvent("GameStartEvent", []()
	{
		PersistentDataManager::GetInstance()->Set("GameState", GameStates::GAME_START);
		SceneManager::GetInstance()->SetActiveGameScene(L"SpaceGameScene");
	});
	pPlayButton->RegisterOnHoverEvent("ButtonHoveredEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuHover.mp3", false, 0.5f); });

	Button* pControlsButton{ new Button(L"CONTROLS", fontPath, DirectX::XMFLOAT4(DirectX::Colors::White)) };
	pControlsButton->SetHoverColor(DirectX::XMFLOAT4(DirectX::Colors::Orange));
	pControlsButton->GetTransform()->Translate(DirectX::XMFLOAT3(250.f, 325.f, 0.f));
	pControlsButton->RegisterOnClickEvent("ButtonClickedEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuClick.mp3"); });
	pControlsButton->RegisterOnClickEvent("ToControlsMenuEvent", []() { SceneManager::GetInstance()->SetActiveGameScene(L"ControlsMenu"); });
	pControlsButton->RegisterOnHoverEvent("ButtonHoveredEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuHover.mp3", false, 0.5f); });

	Button* pCreditButton{ new Button(L"CREDITS", fontPath, DirectX::XMFLOAT4(DirectX::Colors::White)) };
	pCreditButton->SetHoverColor(DirectX::XMFLOAT4(DirectX::Colors::Orange));
	pCreditButton->GetTransform()->Translate(DirectX::XMFLOAT3(250.f, 400.f, 0.f));
	pCreditButton->RegisterOnClickEvent("ButtonClickedEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuClick.mp3"); });
	pCreditButton->RegisterOnClickEvent("ToCreditMenuEvent", []() { SceneManager::GetInstance()->SetActiveGameScene(L"CreditsMenu"); });
	pCreditButton->RegisterOnHoverEvent("ButtonHoveredEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuHover.mp3", false, 0.5f); });

	Button* pQuitButton{ new Button(L"QUIT", fontPath, DirectX::XMFLOAT4(DirectX::Colors::White)) };
	pQuitButton->SetHoverColor(DirectX::XMFLOAT4(DirectX::Colors::Orange));
	pQuitButton->GetTransform()->Translate(DirectX::XMFLOAT3(250.f, 475.f, 0.f));
	pQuitButton->RegisterOnClickEvent("ButtonClickedEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuClick.mp3"); });
	pQuitButton->RegisterOnClickEvent("GameQuitEvent", []() { PostQuitMessage(0); });
	pQuitButton->RegisterOnHoverEvent("ButtonHoveredEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuHover.mp3", false, 0.5f); });
	
	AddChild(pBackground);
	
	AddChild(pPlayButton);
	AddChild(pControlsButton);
	AddChild(pCreditButton);
	AddChild(pQuitButton);
}

void MainMenu::Update()
{
}

void MainMenu::Draw()
{
}