#include "stdafx.h"
#include "InGameMenu.h"

#include "ColliderComponent.h"
#include "ContentManager.h"
#include "FixedCamera.h"
#include "GameObject.h"
#include "HelperStruct.h"
#include "OverlordGame.h"
#include "PersistentDataManager.h"
#include "PhysxManager.h"
#include "PlanetComponent.h"
#include "PostOcean.h"
#include "RigidBodyComponent.h"
#include "SceneManager.h"
#include "SkyBox.h"
#include "SoundManager.h"
#include "SpriteComponent.h"
#include "TransformComponent.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "UI/Button.h"

InGameMenu::InGameMenu()
	: GameScene(L"InGameMenu")
	, m_pCamera(nullptr)
	, m_pPlanet(nullptr)
	, m_pPlanetComponent(nullptr)
	, m_pOceanPostProcessing(nullptr)
	, m_UpdatePlanet{ false }
{}

InGameMenu::~InGameMenu()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void InGameMenu::Initialize()
{
	const auto& gameContext{ GetGameContext() };
	const auto& windowSettings{ OverlordGame::GetGameSettings().Window };
	//INIT IMGUI
//************
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NavEnableSetMousePos | ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos;
	io.WantCaptureMouse = true;
	io.WantSetMousePos = true;
	ImGui_ImplWin32_Init(windowSettings.WindowHandle);
	ImGui_ImplDX11_Init(gameContext.pDevice, gameContext.pDeviceContext);

	FixedCamera* pCamera{ new FixedCamera() };
	AddChild(pCamera);
	m_pCamera = pCamera->GetComponent<CameraComponent>();

	GameObject* pBackground{ new GameObject() };
	SpriteComponent* pBackGroundSprite{ new SpriteComponent(L"Resources/Textures/Game/UI/InGameMenu_ButtonBackground.png") };
	pBackground->AddComponent(pBackGroundSprite);
	pBackground->GetTransform()->Translate(0.f, static_cast<float>(windowSettings.Height) - 65.f, 0.f);

	const std::wstring fontPath{ L"./Resources/Textures/Game/Font/Segoe_Script_Bold_30.fnt" };
	const float menuHeight{ static_cast<float>(windowSettings.Height) - 40.f };

	Button* pResumeButton{ new Button(L"RESUME", fontPath, DirectX::XMFLOAT4(DirectX::Colors::White)) };
	pResumeButton->SetHoverColor(DirectX::XMFLOAT4(DirectX::Colors::Orange));
	pResumeButton->GetTransform()->Translate(65.f, menuHeight, 0.f);
	pResumeButton->RegisterOnClickEvent("ButtonClickedEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuClick.mp3"); });
	pResumeButton->RegisterOnClickEvent("ResumeEvent", []() { SceneManager::GetInstance()->SetActiveGameScene(L"SpaceGameScene"); });
	pResumeButton->RegisterOnHoverEvent("ButtonHoveredEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuHover.mp3", false, 0.5f); });

	Button* pTeleportRandomButton{ new Button(L"RANDOM TELEPORT", fontPath, DirectX::XMFLOAT4(DirectX::Colors::White)) };
	pTeleportRandomButton->SetHoverColor(DirectX::XMFLOAT4(DirectX::Colors::Orange));
	pTeleportRandomButton->GetTransform()->Translate(270.f, menuHeight, 0.f);
	pTeleportRandomButton->RegisterOnClickEvent("ButtonClickedEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuClick.mp3"); });
	pTeleportRandomButton->RegisterOnClickEvent("TeleportEvent", []()
	{
		PersistentDataManager::GetInstance()->Set("GameState", GameStates::TELEPORT);
		PersistentDataManager::GetInstance()->Set("CurrentPlanetDesc", PlanetGenerator::GenerateRandomPlanetDesc());
		SceneManager::GetInstance()->SetActiveGameScene(L"SpaceGameScene");
	});
	pTeleportRandomButton->RegisterOnHoverEvent("ButtonHoveredEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuHover.mp3", false, 0.5f); });

	Button* pTeleportSelectedButton{ new Button(L"TELEPORT TO SELECTED PLANET", fontPath, DirectX::XMFLOAT4(DirectX::Colors::White)) };
	pTeleportSelectedButton->SetHoverColor(DirectX::XMFLOAT4(DirectX::Colors::Orange));
	pTeleportSelectedButton->GetTransform()->Translate(620.f, menuHeight, 0.f);
	pTeleportSelectedButton->RegisterOnClickEvent("ButtonClickedEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuClick.mp3"); });
	pTeleportSelectedButton->RegisterOnClickEvent("TeleportEvent", [this]()
	{
		PersistentDataManager::GetInstance()->Set("GameState", GameStates::TELEPORT);
		PersistentDataManager::GetInstance()->Set("CurrentPlanetDesc", m_pPlanetComponent->GetDesc());
		SceneManager::GetInstance()->SetActiveGameScene(L"SpaceGameScene");
	});
	pTeleportSelectedButton->RegisterOnHoverEvent("ButtonHoveredEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuHover.mp3", false, 0.5f); });

	Button* pBackToMainButton{ new Button(L"BACK TO MAIN MENU", fontPath, DirectX::XMFLOAT4(DirectX::Colors::White)) };
	pBackToMainButton->SetHoverColor(DirectX::XMFLOAT4(DirectX::Colors::Orange));
	pBackToMainButton->GetTransform()->Translate(970.f, menuHeight, 0.f);
	pBackToMainButton->RegisterOnClickEvent("ButtonClickedEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuClick.mp3"); });
	pBackToMainButton->RegisterOnClickEvent("BackToMainEvent", []()
	{
		PersistentDataManager::GetInstance()->Set("GameState", GameStates::BACK_TO_MAIN);
		SceneManager::GetInstance()->SetActiveGameScene(L"SpaceGameScene");
	});
	pBackToMainButton->RegisterOnHoverEvent("ButtonHoveredEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuHover.mp3", false, 0.5f); });

	Button* pQuitButton{ new Button(L"QUIT GAME", fontPath, DirectX::XMFLOAT4(DirectX::Colors::White)) };
	pQuitButton->SetHoverColor(DirectX::XMFLOAT4(DirectX::Colors::Orange));
	pQuitButton->GetTransform()->Translate(1200.f, menuHeight, 0.f);
	pQuitButton->RegisterOnClickEvent("ButtonClickedEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuClick.mp3"); });
	pQuitButton->RegisterOnClickEvent("QuitEvent", []() { PostQuitMessage(0); });
	pQuitButton->RegisterOnHoverEvent("ButtonHoveredEvent", []() { SoundManager::GetInstance()->PlayCachedSound("Resources/Audio/FX/MenuHover.mp3", false, 0.5f); });

	auto* pSkyBox{ new SkyBox(L"Resources/CubeMap/Space.dds") };
	AddChild(pSkyBox);
	
	AddChild(pBackground);
	
	AddChild(pResumeButton);
	AddChild(pTeleportRandomButton);
	AddChild(pTeleportSelectedButton);
	AddChild(pBackToMainButton);
	AddChild(pQuitButton);
}

void InGameMenu::SetNewCurrentPlanet() const
{
	PersistentDataManager::GetInstance()->Set("CurrentPlanetDesc", m_pPlanetComponent->GetDesc());
}

void InGameMenu::Update()
{
	using namespace DirectX;
	const auto& gameContext{ GetGameContext() };
	const float moveSpeed{ 100.f };
	const float rotSpeed{ 0.5f };
	const float deltaT{ gameContext.pGameTime->GetElapsed() };
	
	TransformComponent* pCameraTransform{ m_pCamera->GetTransform() };
	const XMFLOAT3& cameraPos{ pCameraTransform->GetPosition() };

	float move = InputManager::IsKeyboardKeyDown('W') || InputManager::IsKeyboardKeyDown(VK_UP) ? 1.0f : 0.0f;
	if (move == 0) move = -(InputManager::IsKeyboardKeyDown('S') || InputManager::IsKeyboardKeyDown(VK_DOWN) ? 1.0f : 0.0f);

	XMFLOAT3 newPos{};
	XMStoreFloat3(&newPos, XMLoadFloat3(&cameraPos) + XMLoadFloat3(&pCameraTransform->GetForward()) * move * moveSpeed * deltaT);
	pCameraTransform->Translate(newPos);
	
	if (InputManager::IsMouseButtonDown(VK_RBUTTON))
	{		
		TransformComponent* pPlanetTransform{ m_pPlanet->GetTransform() };
		XMFLOAT4 rot{ pPlanetTransform->GetRotation() };
		const auto mouseMove = InputManager::GetMouseMovement();

		XMVECTOR newRotation = XMQuaternionMultiply(XMQuaternionRotationAxis(XMLoadFloat3(&pCameraTransform->GetUp()), XMConvertToRadians(static_cast<float>(mouseMove.x)) * rotSpeed)
			, XMQuaternionRotationAxis(XMLoadFloat3(&pCameraTransform->GetRight()), XMConvertToRadians(static_cast<float>(mouseMove.y)) * rotSpeed));

		newRotation = XMQuaternionMultiply(XMLoadFloat4(&rot), newRotation);
		m_pPlanet->GetTransform()->Rotate(newRotation);
	}

	if (m_UpdatePlanet)
	{
		PlanetGenerator::GetInstance()->Generate(gameContext, m_pPlanetComponent, false);
		m_UpdatePlanet = false;
	}
}

void InGameMenu::DrawNoiseParameterUI(NoiseParameter& np, int idx)
{
	std::string label{ "Noise Count " + std::to_string(idx) + "  : " };

	if (ImGui::SliderFloat(label.c_str(), &np.data.NoiseCount, 0.f, 10.f, "%.0f"))
	{
		m_UpdatePlanet = true;
	}

	label = "Noise Height " + std::to_string(idx) + "  : ";
	if (ImGui::SliderFloat(label.c_str(), &np.data.NoiseHeight, -10.f, 10.f, "%.3f"))
	{
		m_UpdatePlanet = true;
	}

	label = "Noise Frequency " + std::to_string(idx) + "  : ";
	if (ImGui::SliderFloat(label.c_str(), &np.data.Frequency, 0.f, 10.f, "%.3f"))
	{
		m_UpdatePlanet = true;
	}

	label = "Noise Roughness " + std::to_string(idx) + "  : ";
	if (ImGui::SliderFloat(label.c_str(), &np.data.FrequencyMultiplier, 1.f, 20.f, "%.2f"))
	{
		m_UpdatePlanet = true;
	}

	label = "Noise Amplitude " + std::to_string(idx) + "  : ";
	if (ImGui::SliderFloat(label.c_str(), &np.data.Amplitude, 0.f, 1.f, "%.4f"))
	{
		m_UpdatePlanet = true;
	}

	label = "Noise Offset " + std::to_string(idx) + "  : ";
	if (ImGui::SliderFloat(label.c_str(), &np.data.Offset, 0.f, 5.f, "%.3f"))
	{
		m_UpdatePlanet = true;
	}

	label = "Noise v-shift " + std::to_string(idx) + "  : ";
	if (ImGui::SliderFloat(label.c_str(), &np.data.VerticalShift, -50.f, 50.f, "%.2f"))
	{
		m_UpdatePlanet = true;
	}
}


void InGameMenu::Draw()
{
	//DISPLAY THE "Planet Editor" IMGUI MENU - is hidden behind the Skybox somehow

	const auto mousPos{ InputManager::GetMousePosition() };
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(static_cast<float>(mousPos.x), static_cast<float>(mousPos.y));
	io.MouseDown[0] = InputManager::IsMouseButtonDown(VK_LBUTTON);
	io.MouseDown[2] = InputManager::IsMouseButtonDown(VK_RBUTTON);
	
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	ImGui::Begin("Planet Generator - Have a go !");
	ImGui::SetWindowPos({ 0.f, 0.f });
	ImGui::SetWindowSize({ 350.f, static_cast<float>(OverlordGame::GetGameSettings().Window.Height) - 65.f });

	ImGui::Text("SHADER PARAMETERS");
	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

	PlanetGenDescription& planetDesc{ m_pPlanetComponent->GetDesc() };
	
	if (ImGui::SliderInt("Subdivision (LOD): ", &planetDesc.Subdivision, 1, 64, "%d"))
	{
		m_UpdatePlanet = true;
	}
	
	if (ImGui::SliderInt("Planet Scale: ", &planetDesc.Scaling, 1, 200, "%d"))
	{
		m_UpdatePlanet = true;
	}
	
	if (ImGui::SliderFloat("Max Ocean Depth: ", &planetDesc.OceanDepth, 1.f, 50.f, "%.4f"))
	{
		m_UpdatePlanet = true;
	}
	
	if (ImGui::SliderFloat("Ocean Depth Multiplier: ", &planetDesc.OceanDepthMultiplier, 1.f, 50.f, "%.1f"))
	{
		m_UpdatePlanet = true;
	}

	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	ImGui::Text("Base Shape");
	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	DrawNoiseParameterUI(planetDesc.BaseShapeParams, 0);

	ImGui::Text("Detail Noise");
	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	DrawNoiseParameterUI(planetDesc.DetailParams, 1);

	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	ImGui::Text("Ridged Noise");
	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	DrawNoiseParameterUI(planetDesc.MountainParams, 2);

	//ImGui::Text("Ridged Mask");
	//ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	//DrawNoiseParameterUI(planetDesc.MountainMaskParams, 3);

	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	ImGui::Text("Colors");
	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	const ImGuiColorEditFlags colorPickerFlags{ ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoAlpha };
	ImGui::ColorEdit3("UnderWater", &planetDesc.ColorSettings.UnderwaterGroundColor.x, colorPickerFlags);
	ImGui::ColorEdit3("Shore", &planetDesc.ColorSettings.ShoreColor.x, colorPickerFlags);
	ImGui::ColorEdit3("Low Plane", &planetDesc.ColorSettings.PlaneColorLow.x, colorPickerFlags);
	ImGui::ColorEdit3("High Plane", &planetDesc.ColorSettings.PlaneColorHigh.x, colorPickerFlags);
	ImGui::ColorEdit3("Heights", &planetDesc.ColorSettings.HeightsColor.x, colorPickerFlags);
	ImGui::Spacing();
	ImGui::ColorEdit3("Ocean1", &planetDesc.OceanColorShallow.x, colorPickerFlags);
	ImGui::ColorEdit3("Ocean2", &planetDesc.OceanColorDeep.x, colorPickerFlags);
	
	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void InGameMenu::SceneActivated()
{
	using namespace DirectX;
	const auto& gameContext{ GetGameContext() };

	PlanetGenDescription planetDesc{};
	if (PersistentDataManager::GetInstance()->Get("CurrentPlanetDesc", planetDesc))
	{
		const float sunDistance{ 300.f };
		XMFLOAT3 sunDir{ -0.577f, -0.577f, 0.577f };
		XMFLOAT3 sunPos;
		XMStoreFloat3(&sunPos, -XMLoadFloat3(&sunDir) * sunDistance);

		gameContext.pShadowMapper->SetLight(sunPos, sunDir);
		gameContext.pShadowMapper->UpdateLightViewSettings(550.f, 0.01f, sunDistance * 2.f);
		gameContext.pShadowMapper->SetShadowIllumination(0.3f);

		auto* physX = PhysxManager::GetInstance()->GetPhysics();

		m_pPlanet = new GameObject();
		m_pPlanetComponent = new PlanetComponent(planetDesc);
		m_pPlanet->AddComponent(m_pPlanetComponent);

		RigidBodyComponent* pBody{ new RigidBodyComponent() };
		pBody->SetKinematic(true);
		physx::PxTriangleMesh* pTriangleMesh{ ContentManager::Load<physx::PxTriangleMesh>(L"Resources/Meshes/icosahedron.ovpt") };
		std::shared_ptr<physx::PxGeometry> pGeo{ std::make_shared<physx::PxTriangleMeshGeometry>(pTriangleMesh) };
		const auto* pNullPhysxMat = physX->createMaterial(0.f, 0.f, 0.f);
		ColliderComponent* pCollider{ new ColliderComponent(pGeo, *pNullPhysxMat) };
		pBody->SetCollisionGroup(CollisionGroupFlag::Group1);
		m_pPlanet->AddComponent(pBody);
		m_pPlanet->AddComponent(pCollider);
		AddChild(m_pPlanet);

		PlanetGenerator::GetInstance()->Generate(gameContext, m_pPlanetComponent);

		m_pOceanPostProcessing = new PostOcean(m_pPlanetComponent);
		AddPostProcessingEffect(m_pOceanPostProcessing);

		m_pCamera->SetActive();
		m_pCamera->GetTransform()->Translate(-100.f, 0.f, -650.f);
	}
	else
		Logger::LogError(L"InGameMenu::SceneActivated > Could not retrieve current planet description!");
}

void InGameMenu::SceneDeactivated()
{
	RemovePostProcessingEffect(m_pOceanPostProcessing);
	RemoveChild(m_pPlanet, true);
}