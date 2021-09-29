//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ShadowMapMaterial.h"
#include "ContentManager.h"

ShadowMapMaterial::~ShadowMapMaterial()
{
	//TODO: make sure you don't have memory leaks and/or resource leaks :) -> Figure out if you need to do something here
	for (uint32_t idx{}; idx < NUM_TYPES; ++idx)
	{
		SafeRelease(m_pInputLayouts[idx]);
	}
}

void ShadowMapMaterial::Initialize(const GameContext& gameContext)
{
	if (!m_IsInitialized)
	{
		//TODO: initialize the effect, techniques, shader variables, input layouts (hint use EffectHelper::BuildInputLayout), etc.
		m_pShadowEffect = ContentManager::Load<ID3DX11Effect>(L"./Resources/Effects/ShadowMapGenerator.fx");

		m_pWorldMatrixVariable = m_pShadowEffect->GetVariableByName("gWorld")->AsMatrix();
		if (!m_pWorldMatrixVariable->IsValid())
		{
			Logger::LogError(L"ShadowMapMaterial::Initialize() > Shader variable \'gWorld\' not valid!");
			m_pWorldMatrixVariable = nullptr;
			return;
		}

		m_pBoneTransforms = m_pShadowEffect->GetVariableByName("gBones")->AsMatrix();
		if (!m_pBoneTransforms->IsValid())
		{
			Logger::LogError(L"ShadowMapMaterial::Initialize() > Shader variable \'gBones\' not valid!");
			m_pBoneTransforms = nullptr;
			return;
		}

		m_pLightVPMatrixVariable = m_pShadowEffect->GetVariableByName("gLightViewProj")->AsMatrix();
		if (!m_pLightVPMatrixVariable->IsValid())
		{
			Logger::LogError(L"ShadowMapMaterial::Initialize() > Shader variable \'gLightViewProj\' not valid!");
			m_pLightVPMatrixVariable = nullptr;
			return;
		}

		auto* pDevice{ gameContext.pDevice };
		for (uint32_t idx{}; idx < NUM_TYPES; ++idx)
		{
			m_pShadowTechs[idx] = m_pShadowEffect->GetTechniqueByIndex(idx);
			if (!EffectHelper::BuildInputLayout(pDevice, m_pShadowTechs[idx], &m_pInputLayouts[idx]
				, m_InputLayoutDescriptions[idx], m_InputLayoutSizes[idx], m_InputLayoutIds[idx]))
			{
				Logger::LogError(L"ShadowMapMaterial::Initialize() > Input layout" + std::to_wstring(idx) + L" could not be built !");
				return;
			}
		}

		m_IsInitialized = true;
	}
}

void ShadowMapMaterial::SetLightVP(DirectX::XMFLOAT4X4 lightVP) const
{
	//TODO: set the correct shader variable
	if (m_pLightVPMatrixVariable)
		m_pLightVPMatrixVariable->SetMatrix(lightVP.m[0]);
}

void ShadowMapMaterial::SetWorld(DirectX::XMFLOAT4X4 world) const
{
	//TODO: set the correct shader variable
	if (m_pWorldMatrixVariable)
		m_pWorldMatrixVariable->SetMatrix(world.m[0]);
}

void ShadowMapMaterial::SetBones(const float* pData, int count) const
{
	//TODO: set the correct shader variable
	if (m_pBoneTransforms)
		m_pBoneTransforms->SetMatrixArray(pData, 0, count);
}
