#include "stdafx.h"
#include "PlanetComponent.h"
#include "ContentManager.h"
#include "GameObject.h"
#include "PlanetGenerator.h"
#include "RigidBodyComponent.h"
#include "TransformComponent.h"
#include "../../Materials/PlanetMaterials/PlanetMaterial.h"

PlanetComponent::PlanetComponent(bool random) :
	PlanetComponent(InitGenDesc(random))
{}

PlanetComponent::PlanetComponent(PlanetGenDescription genDesc) :
	m_PlanetGenDesc(genDesc)
	, m_pVertexBuffer(nullptr)
	, m_pMaterial(nullptr)
	, m_Vertices{}
	, m_TriangleCount(0)
	, m_MaxHeight(0.f)
	, m_MinHeight(0.f)
	, m_Gravity(randF(-50.f, -15.f))
	, m_CastShadows(true)
	, m_IsInitialized(false)
{}

PlanetComponent::~PlanetComponent()
{
	SafeRelease(m_pVertexBuffer);
	SafeDelete(m_pMaterial);
}

void PlanetComponent::Initialize(const GameContext& gameContext)
{
	m_pMaterial = new PlanetMaterial();
	m_pMaterial->Initialize(gameContext);

	D3D11_BUFFER_DESC vBufferDesc{};
	vBufferDesc.ByteWidth = m_pMaterial->GetLayoutSize() * PlanetGenerator::MAXVERTEXOUTPUT;
	vBufferDesc.CPUAccessFlags = 0;
	vBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vBufferDesc.BindFlags = D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_VERTEX_BUFFER;
	vBufferDesc.MiscFlags = 0;

	const auto result = gameContext.pDevice->CreateBuffer(&vBufferDesc, nullptr, &m_pVertexBuffer);
	Logger::LogHResult(result, L"PlanetComponent::Initialize > Could not create vertex buffer!");
};

DirectX::XMFLOAT4 PlanetComponent::GetFrameRotation(float deltaTime) const
{
	DirectX::XMVECTOR rot{ DirectX::XMQuaternionRotationAxis(XMLoadFloat3(&m_PlanetGenDesc.RotationAxis), m_PlanetGenDesc.RotationSpeed * deltaTime) };

	DirectX::XMFLOAT4 frameRot{};
	XMStoreFloat4(&frameRot, rot);
	return frameRot;
}

void PlanetComponent::Update(const GameContext& gameContext)
{
	TransformComponent* t{ GetTransform() };
	DirectX::XMFLOAT4 frameRot{ GetFrameRotation(gameContext.pGameTime->GetElapsed()) };

	const DirectX::XMVECTOR xmvFrameRot{ XMLoadFloat4(&frameRot) };
	DirectX::XMVECTOR rotation{
		DirectX::XMQuaternionMultiply(XMLoadFloat4(&t->GetRotation())
		, xmvFrameRot) };

	t->Rotate(rotation);
}

void PlanetComponent::DrawShadowMap(const GameContext& gameContext)
{
	if (!m_CastShadows)
		return;

	gameContext.pShadowMapper->DrawAuto(gameContext, m_pVertexBuffer, m_pMaterial->GetLayoutSize(), GetTransform()->GetWorld());
};

void PlanetComponent::Draw(const GameContext& gameContext)
{
	if (!m_pMaterial)
	{
		Logger::LogWarning(L"PlanetComponent::Draw() > No Material!");
		return;
	}

	m_pMaterial->UpdateEffectVariables(gameContext, this);
	//Set Inputlayout
	gameContext.pDeviceContext->IASetInputLayout(m_pMaterial->GetInputLayout());
	//Set Vertex Buffer
	UINT stride = m_pMaterial->GetLayoutSize();
	UINT offset = 0;
	gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	gameContext.pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	gameContext.pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//DRAW
	auto tech = m_pMaterial->GetTechnique();
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, gameContext.pDeviceContext);
		gameContext.pDeviceContext->DrawAuto();
	}
}

PlanetGenDescription PlanetComponent::InitGenDesc(bool random)
{
	PlanetGenDescription genDesc{};
	if (random)
	{
		genDesc = PlanetGenerator::GenerateRandomPlanetDesc();
	}
	else
	{
		//Temporary - will be auto generated later
		genDesc.Subdivision = 64;
		genDesc.Scaling = 200;
		genDesc.BaseShapeParams = { 2.49f, 6.f, 0.95f, 2.76f, 0.3812f, 0.00f, 0.f };
		genDesc.DetailParams = { 2.958f, 4.f, 2.817f, 6.71f, 0.5023f, 2.981f, 2.58f };
		genDesc.MountainParams = { 5.12f, 3.f, 1.75f, 4.78f, 0.326f, 2.845f, 0.f };
		genDesc.MountainMaskParams = {};
		genDesc.OceanDepth = 15.f;
		genDesc.OceanDepthMultiplier = 12.f;
		genDesc.ColorSettings.UnderwaterGroundColor = DirectX::XMFLOAT3{ 0.368f, 0.298f, 0.211f };
		genDesc.ColorSettings.ShoreColor = DirectX::XMFLOAT3{ 0.6f, 0.55f, 0.35f };
		genDesc.ColorSettings.PlaneColorLow = DirectX::XMFLOAT3{ 0.3f, 0.5f, 0.25f };
		genDesc.ColorSettings.PlaneColorHigh = DirectX::XMFLOAT3{ 0.20f, 0.325f, 0.20f };
		genDesc.ColorSettings.HeightsColor = DirectX::XMFLOAT3{ 0.368f, 0.298f, 0.211f };
		genDesc.ColorSettings.ShoreColorSettings = DirectX::XMFLOAT3{ 0.01f, -0.f, 0.01f };
		genDesc.ColorSettings.PlaneColorSettings = DirectX::XMFLOAT3{ 0.4f, -0.1f, 0.1f };
		genDesc.OceanColorShallow = DirectX::XMFLOAT3{ 0.345f, 0.929f, 0.921f };
		genDesc.OceanColorDeep = DirectX::XMFLOAT3{ 0.066f, 0.207f, 0.451f };
		genDesc.RotationAxis = DirectX::XMFLOAT3{ 0.f, 1.f, 0.f };
		genDesc.RotationSpeed = DirectX::XMConvertToRadians(0.75f);
		genDesc.ResourceCount = 200;
	}

	return genDesc;
}
