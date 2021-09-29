#include "stdafx.h"
#include "PostOcean.h"


#include "ContentManager.h"
#include "PlanetComponent.h"
#include "RenderTarget.h"
#include "TextureData.h"
#include "TransformComponent.h"

PostOcean::PostOcean(PlanetComponent* pPlanet)
	: PostProcessingMaterial(L"./Resources/Effects/Post/Ocean.fx", 1),
	m_pTextureMapVariable(nullptr),
	m_pDepthTextureMapVariable(nullptr),
	m_pNormalMapVariable(nullptr),
	m_pProjInverse(nullptr),
	m_pViewInverse(nullptr),
	m_pWorldMatrix(nullptr),
	m_pPlanetRadius(nullptr),
	m_pTimeVariable(nullptr),
	m_pShallowColor(nullptr),
	m_pDeepColor(nullptr),
	m_pMaxDepth(nullptr),
	m_pPlanet(pPlanet)
{
}

void PostOcean::LoadEffectVariables()
{
	//TODO: Bind the 'gTexture' variable with 'm_pTextureMapVariable'
	//Check if valid!
	if (!m_pTextureMapVariable)
	{
		m_pTextureMapVariable = GetEffect()->GetVariableByName("gColorTexture")->AsShaderResource();
		if (!m_pTextureMapVariable->IsValid())
		{
			Logger::LogWarning(L"PostOcean::LoadEffectVariables() > \'gColorTexture\' variable not found!");
			m_pTextureMapVariable = nullptr;
		}
	}

	if (!m_pDepthTextureMapVariable)
	{
		m_pDepthTextureMapVariable = GetEffect()->GetVariableByName("gDepthTexture")->AsShaderResource();
		if (!m_pDepthTextureMapVariable->IsValid())
		{
			Logger::LogWarning(L"PostOcean::LoadEffectVariables() > \'gDepthTexture\' variable not found!");
			m_pDepthTextureMapVariable = nullptr;
		}
	}

	if (!m_pNormalMapVariable)
	{
		m_pNormalMapVariable = GetEffect()->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapVariable->IsValid())
		{
			Logger::LogWarning(L"PostOcean::LoadEffectVariables() > \'gNormalMap\' variable not found!");
			m_pNormalMapVariable = nullptr;
		}
	}

	if (!m_pProjInverse)
	{
		m_pProjInverse = GetEffect()->GetVariableByName("gProjInverse")->AsMatrix();
		if (!m_pProjInverse->IsValid())
		{
			Logger::LogWarning(L"PostOcean::LoadEffectVariables() > \'gProjInverse\' variable not found!");
			m_pProjInverse = nullptr;
		}
	}

	if (!m_pViewInverse)
	{
		m_pViewInverse = GetEffect()->GetVariableByName("gViewInverse")->AsMatrix();
		if (!m_pViewInverse->IsValid())
		{
			Logger::LogWarning(L"PostOcean::LoadEffectVariables() > \'gViewInverse\' variable not found!");
			m_pViewInverse = nullptr;
		}
	}
	
	if (!m_pWorldMatrix)
	{
		m_pWorldMatrix = GetEffect()->GetVariableByName("gWorld")->AsMatrix();
		if (!m_pWorldMatrix->IsValid())
		{
			Logger::LogWarning(L"PostOcean::LoadEffectVariables() > \'gWorld\' variable not found!");
			m_pWorldMatrix = nullptr;
		}
	}

	if (!m_pTimeVariable)
	{
		m_pTimeVariable = GetEffect()->GetVariableByName("gTime")->AsScalar();
		if (!m_pTimeVariable->IsValid())
		{
			Logger::LogWarning(L"PostOcean::LoadEffectVariables() > \'gTime\' variable not found!");
			m_pTimeVariable = nullptr;
		}
	}

	if (!m_pShallowColor)
	{
		m_pShallowColor = GetEffect()->GetVariableByName("gShallowColor")->AsVector();
		if (!m_pShallowColor->IsValid())
		{
			Logger::LogWarning(L"PostOcean::LoadEffectVariables() > \'gShallowColor\' variable not found!");
			m_pShallowColor = nullptr;
		}
	}

	if (!m_pDeepColor)
	{
		m_pDeepColor = GetEffect()->GetVariableByName("gDeepColor")->AsVector();
		if (!m_pDeepColor->IsValid())
		{
			Logger::LogWarning(L"PostOcean::LoadEffectVariables() > \'gDeepColor\' variable not found!");
			m_pDeepColor = nullptr;
		}
	}

	if (!m_pMaxDepth)
	{
		m_pMaxDepth = GetEffect()->GetVariableByName("gMaxDepth")->AsScalar();
		if (!m_pMaxDepth->IsValid())
		{
			Logger::LogWarning(L"PostOcean::LoadEffectVariables() > \'gMaxDepth\' variable not found!");
			m_pMaxDepth = nullptr;
		}
	}
	
	if (!m_pPlanetRadius)
	{
		m_pPlanetRadius = GetEffect()->GetVariableByName("gPlanetRadius")->AsScalar();
		if (!m_pPlanetRadius->IsValid())
		{
			Logger::LogWarning(L"PostOcean::LoadEffectVariables() > \'gPlanetRadius\' variable not found!");
			m_pPlanetRadius = nullptr;
		}
	}
}

void PostOcean::UpdateEffectVariables(const GameContext& gameContext, RenderTarget* pRendertarget)
{
	const auto& desc{ m_pPlanet->GetDesc() };
	
	if (m_pTextureMapVariable)
		m_pTextureMapVariable->SetResource(pRendertarget->GetShaderResourceView());

	if (m_pDepthTextureMapVariable)
		m_pDepthTextureMapVariable->SetResource(pRendertarget->GetDepthShaderResourceView());

	if (m_pNormalMapVariable)
	{
		auto* normalTexture = ContentManager::Load<TextureData>(L"Resources/Textures/Game/WaveNormalMap.png");
		m_pNormalMapVariable->SetResource(normalTexture->GetShaderResourceView());
	}

	if (m_pProjInverse)
	{
		const DirectX::XMFLOAT4X4 proj{ gameContext.pCamera->GetProjection() };
		DirectX::XMFLOAT4X4 projInverse{};
		XMStoreFloat4x4(&projInverse, XMMatrixInverse(nullptr, XMLoadFloat4x4(&proj)));
		m_pProjInverse->SetMatrix(projInverse.m[0]);
	}

	if (m_pViewInverse)
		m_pViewInverse->SetMatrix(gameContext.pCamera->GetViewInverse().m[0]);

	if (m_pWorldMatrix)
		m_pWorldMatrix->SetMatrix(m_pPlanet->GetTransform()->GetWorld().m[0]);
	
	if (m_pPlanetRadius)
		m_pPlanetRadius->SetFloat(static_cast<float>(m_pPlanet->GetDesc().Scaling));

	const float timeScale{ 0.05f };
	if (m_pTimeVariable)
		m_pTimeVariable->SetFloat(gameContext.pGameTime->GetTotal() * timeScale);

	if (m_pShallowColor)
		m_pShallowColor->SetFloatVector(&desc.OceanColorShallow.x);

	if (m_pDeepColor)
		m_pDeepColor->SetFloatVector(&desc.OceanColorDeep.x);

	if (m_pMaxDepth)
		m_pMaxDepth->SetFloat(abs(m_pPlanet->GetMinHeight()));
}

void PostOcean::UnbindEffectVariables(const GameContext& gameContext)
{
	const int srvCount{ 2 };
	ID3D11ShaderResourceView* nullSRV[] = { nullptr, nullptr };
	gameContext.pDeviceContext->PSSetShaderResources(0, srvCount, nullSRV);
}