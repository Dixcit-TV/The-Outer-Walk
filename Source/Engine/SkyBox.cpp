#include "stdafx.h"
#include "SkyBox.h"

#include "ModelComponent.h"
#include "SkyBoxMaterial.h"

SkyBox::SkyBox(const std::wstring& cubeMapPath)
	: GameObject()
	, m_CubeMapPath(cubeMapPath)
{}

void SkyBox::Initialize(const GameContext& gameContext)
{
	ModelComponent* pCube = new ModelComponent(L"Resources/Meshes/Box.ovm");
	SkyBoxMaterial* pMaterial{ new SkyBoxMaterial() };
	pMaterial->SetCubeMap(m_CubeMapPath);
	gameContext.pMaterialManager->AddMaterial(pMaterial, 999);
	pCube->SetMaterial(999);

	AddComponent(pCube);
}