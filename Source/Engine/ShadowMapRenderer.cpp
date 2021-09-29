#include "stdafx.h"
#include "ShadowMapRenderer.h"
#include "ContentManager.h"
#include "GameObject.h"
#include "ShadowMapMaterial.h"
#include "RenderTarget.h"
#include "MeshFilter.h"
#include "SceneManager.h"
#include "OverlordGame.h"

ShadowMapRenderer::~ShadowMapRenderer()
{
	//TODO: make sure you don't have memory leaks and/or resource leaks :) -> Figure out if you need to do something here
	SafeDelete(m_pShadowMat);
	SafeDelete(m_pShadowRT);
}

void ShadowMapRenderer::Initialize(const GameContext& gameContext)
{
	if (m_IsInitialized)
		return;

	//TODO: create shadow generator material + initialize it
	m_pShadowMat = new ShadowMapMaterial();
	m_pShadowMat->Initialize(gameContext);
	
	//TODO: create a rendertarget with the correct settings (hint: depth only) for the shadow generator using a RENDERTARGET_DESC
	m_pShadowRT = new RenderTarget(gameContext.pDevice);

	const auto windowSettings = OverlordGame::GetGameSettings().Window;
	RENDERTARGET_DESC rtDesc{};
	rtDesc.Width = windowSettings.Width;
	rtDesc.Height = windowSettings.Height;
	rtDesc.EnableDepthBuffer = true;
	rtDesc.EnableDepthSRV = true;
	rtDesc.EnableColorBuffer = false;
	rtDesc.EnableColorSRV = false;
	rtDesc.GenerateMipMaps_Color = false;
	m_pShadowRT->Create(rtDesc);

	m_IsInitialized = true;
}

void ShadowMapRenderer::UpdateLightViewSettings(float size, float nearPlane, float farPlane)
{
	bool update = false;
	if (size >= 0.f)
	{
		update = true;
		m_Size = size;
	}

	if (nearPlane >= 0.f)
	{
		update = true;
		m_NearPlane = nearPlane;
	}

	if (farPlane > 0.f && farPlane > m_NearPlane)
	{
		update = true;
		m_FarPlane = farPlane;
	}

	if (update)
		SetLight(m_LightPosition, m_LightDirection);
}

void ShadowMapRenderer::SetLight(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction)
{
	//TODO: store the input parameters in the appropriate datamembers
	m_LightPosition = position;
	m_LightDirection = direction;
	//TODO: calculate the Light VP matrix (Directional Light only ;)) and store it in the appropriate datamember
	using namespace DirectX;
	
	const auto windowSettings = OverlordGame::GetGameSettings().Window;
	const float viewWidth = (m_Size > 0.f) ? m_Size * windowSettings.AspectRatio : static_cast<float>(windowSettings.Width);
	const float viewHeight = (m_Size > 0.f) ? m_Size : static_cast<float>(windowSettings.Height);
	const XMMATRIX projection = XMMatrixOrthographicLH(viewWidth, viewHeight, m_NearPlane, m_FarPlane);

	const XMVECTOR worldPosition = XMLoadFloat3(&m_LightPosition);
	const XMVECTOR lookAt = XMLoadFloat3(&m_LightDirection);
	const XMVECTOR upVec = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	const XMMATRIX view = XMMatrixLookAtLH(worldPosition, worldPosition + lookAt, upVec);

	XMStoreFloat4x4(&m_LightVP, view * projection);
}

void ShadowMapRenderer::Begin(const GameContext& gameContext) const
{
	//Reset Texture Register 5 (Unbind)
	ID3D11ShaderResourceView *const pSRV[] = { nullptr };
	gameContext.pDeviceContext->PSSetShaderResources(1, 1, pSRV);

	//TODO: set the appropriate render target that our shadow generator will write to (hint: use the OverlordGame::SetRenderTarget function through SceneManager)
	SceneManager::GetInstance()->GetGame()->SetRenderTarget(m_pShadowRT);
	//TODO: clear this render target
	m_pShadowRT->Clear(gameContext, nullptr);
	//TODO: set the shader variables of this shadow generator material
	m_pShadowMat->SetLightVP(m_LightVP);
}

void ShadowMapRenderer::End(const GameContext& gameContext) const
{
	UNREFERENCED_PARAMETER(gameContext);
	//TODO: restore default render target (hint: passing nullptr to OverlordGame::SetRenderTarget will do the trick)
	SceneManager::GetInstance()->GetGame()->SetRenderTarget(nullptr);
}

void ShadowMapRenderer::Draw(const GameContext& gameContext, MeshFilter* pMeshFilter, const DirectX::XMFLOAT4X4& world, const std::vector<DirectX::XMFLOAT4X4>& bones) const
{
	//TODO: update shader variables in material
	m_pShadowMat->SetWorld(world);
	m_pShadowMat->SetBones(std::data(bones)->m[0], bones.size());

	const size_t layoutId = static_cast<size_t>(pMeshFilter->m_HasAnimations);
	auto* pDeviceContext{ gameContext.pDeviceContext };
	//TODO: set the correct inputlayout, buffers, topology (some variables are set based on the generation type Skinned or Static)

	VertexBufferData vBufferData{ pMeshFilter->GetVertexBufferData(gameContext, m_pShadowMat->m_InputLayoutIds[layoutId]) };
	UINT stride{ vBufferData.VertexStride };
	UINT offset{ 0 };
	pDeviceContext->IASetVertexBuffers(0, 1, &vBufferData.pVertexBuffer, &stride, &offset);

	pDeviceContext->IASetIndexBuffer(pMeshFilter->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	pDeviceContext->IASetInputLayout(m_pShadowMat->m_pInputLayouts[layoutId]);

	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//TODO: invoke draw call
	D3DX11_TECHNIQUE_DESC techDesc;
	auto* pTechnique{ m_pShadowMat->m_pShadowTechs[layoutId] };
	pTechnique->GetDesc(&techDesc);

	for (UINT p{ 0 }; p < techDesc.Passes; ++p)
	{
		pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(pMeshFilter->m_IndexCount, 0, 0);
	}
}

void ShadowMapRenderer::DrawAuto(const GameContext& gameContext, ID3D11Buffer* pVBuffer, UINT stride, const DirectX::XMFLOAT4X4& world, const std::vector<DirectX::XMFLOAT4X4>& bones) const
{
	//TODO: update shader variables in material
	m_pShadowMat->SetWorld(world);
	m_pShadowMat->SetBones(std::data(bones)->m[0], bones.size());

	const size_t layoutId = static_cast<size_t>(bones.size() > 0);
	auto* pDeviceContext{ gameContext.pDeviceContext };
	//TODO: set the correct inputlayout, buffers, topology (some variables are set based on the generation type Skinned or Static)

	UINT offset{ 0 };
	pDeviceContext->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
	pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	pDeviceContext->IASetInputLayout(m_pShadowMat->m_pInputLayouts[layoutId]);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//TODO: invoke draw call
	D3DX11_TECHNIQUE_DESC techDesc;
	auto* pTechnique{ m_pShadowMat->m_pShadowTechs[layoutId] };
	pTechnique->GetDesc(&techDesc);

	for (UINT p{ 0 }; p < techDesc.Passes; ++p)
	{
		pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawAuto();
	}
}

void ShadowMapRenderer::UpdateMeshFilter(const GameContext& gameContext, MeshFilter* pMeshFilter) const
{
	//TODO: based on the type (Skinned or Static) build the correct vertex buffers for the MeshFilter (Hint use MeshFilter::BuildVertexBuffer)
	const size_t layoutId = static_cast<size_t>(pMeshFilter->m_HasAnimations);
	pMeshFilter->BuildVertexBuffer(gameContext, m_pShadowMat->m_InputLayoutIds[layoutId], m_pShadowMat->m_InputLayoutSizes[layoutId], m_pShadowMat->m_InputLayoutDescriptions[layoutId]);
}

ID3D11ShaderResourceView* ShadowMapRenderer::GetShadowMap() const
{
	//TODO: return the depth shader resource view of the shadow generator render target
	return m_pShadowRT->GetDepthShaderResourceView();
}
