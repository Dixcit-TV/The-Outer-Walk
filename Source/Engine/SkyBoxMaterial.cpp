#include "stdafx.h"
#include "SkyBoxMaterial.h"

#include "ContentManager.h"
#include "TextureData.h"

SkyBoxMaterial::SkyBoxMaterial() :
	Material(L"./Resources/Effects/SkyBox.fx")
	, m_pCubeMapTexture(nullptr)
	, m_pCubeMapSRVariable(nullptr)
{}

void SkyBoxMaterial::SetCubeMap(const std::wstring& assetFile)
{
	m_pCubeMapTexture = ContentManager::Load<TextureData>(assetFile);
}

void SkyBoxMaterial::LoadEffectVariables()
{
	if (!m_pCubeMapSRVariable)
	{
		m_pCubeMapSRVariable = GetEffect()->GetVariableByName("m_CubeMap")->AsShaderResource();
		if (!m_pCubeMapSRVariable->IsValid())
		{
			Logger::LogWarning(L"SkyBoxMaterial::LoadEffectVariables() > \'m_CubeMap\' variable not found!");
			m_pCubeMapSRVariable = nullptr;
		}
	}
}

void SkyBoxMaterial::UpdateEffectVariables(const GameContext&, ModelComponent*)
{
	if (m_pCubeMapSRVariable && m_pCubeMapTexture)
	{
		m_pCubeMapSRVariable->SetResource(m_pCubeMapTexture->GetShaderResourceView());
	}
}