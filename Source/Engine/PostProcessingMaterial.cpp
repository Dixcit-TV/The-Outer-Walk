#include "stdafx.h"
#include "PostProcessingMaterial.h"
#include "RenderTarget.h"
#include "OverlordGame.h"
#include "ContentManager.h"

PostProcessingMaterial::PostProcessingMaterial(std::wstring effectFile, unsigned int renderIndex,
                                               std::wstring technique)
	: m_IsInitialized(false), 
	  m_pInputLayout(nullptr),
	  m_pInputLayoutSize(0),
	  m_effectFile(std::move(effectFile)),
	  m_InputLayoutID(0),
	  m_RenderIndex(renderIndex),
	  m_pRenderTarget(nullptr),
	  m_pVertexBuffer(nullptr),
	  m_NumVertices(0),
	  m_NumIndices(0),
	  m_VertexBufferStride(0),
	  m_pEffect(nullptr),
	  m_pTechnique(nullptr),
	  m_TechniqueName(std::move(technique))
{
}

PostProcessingMaterial::~PostProcessingMaterial()
{
	//TODO: delete and/or release necessary objects and/or resources
	SafeRelease(m_pVertexBuffer);
	SafeRelease(m_pIndexBuffer);
	SafeRelease(m_pInputLayout);
	SafeDelete(m_pRenderTarget);
}

void PostProcessingMaterial::Initialize(const GameContext& gameContext)
{
	if (!m_IsInitialized)
	{
		//TODO: complete
		LoadEffect(gameContext);
		
		if (!EffectHelper::BuildInputLayout(gameContext.pDevice, m_pTechnique, &m_pInputLayout, m_pInputLayoutDescriptions, m_pInputLayoutSize, m_InputLayoutID))
			Logger::LogError(L"PostProcessingMaterial::Initialize > Could not Build InputLayout !");
		
		CreateVertexBuffer(gameContext);
		CreateInputBuffer(gameContext);
		const auto windowSettings = OverlordGame::GetGameSettings().Window;
		RENDERTARGET_DESC rtDesc{};
		rtDesc.Width = windowSettings.Width;
		rtDesc.Height = windowSettings.Height;
		rtDesc.EnableDepthBuffer = true;
		rtDesc.EnableDepthSRV = true;
		rtDesc.EnableColorBuffer = true;
		rtDesc.EnableColorSRV = true;
		rtDesc.GenerateMipMaps_Color = true;

		m_pRenderTarget = new RenderTarget(gameContext.pDevice);
		const auto hr = m_pRenderTarget->Create(rtDesc);
		if (Logger::LogHResult(hr, L"PostProcessingMaterial::Initialize > Could not create Render Target!"))
			return;

		m_IsInitialized = true;
	}
}

bool PostProcessingMaterial::LoadEffect(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);

	//TODO: complete
	m_pEffect = ContentManager::Load<ID3DX11Effect>(m_effectFile);
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	const std::string techString = converter.to_bytes(m_TechniqueName);
	m_pTechnique = m_pEffect->GetTechniqueByName(techString.c_str());
	if (!m_pTechnique->IsValid())
		m_pTechnique = m_pEffect->GetTechniqueByIndex(0);

	LoadEffectVariables();

	return true;
}

void PostProcessingMaterial::Draw(const GameContext& gameContext, RenderTarget* previousRendertarget)
{
	//TODO: complete
	m_pRenderTarget->Clear(gameContext, reinterpret_cast<const float*>(&DirectX::Colors::Black));
	UpdateEffectVariables(gameContext, previousRendertarget);
	gameContext.pDeviceContext->IASetInputLayout(m_pInputLayout);
	UINT offsets[]{0};
	gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertexBufferStride, offsets);
	gameContext.pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gameContext.pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for (uint32_t p{}; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, gameContext.pDeviceContext);
		gameContext.pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}

	UnbindEffectVariables(gameContext); //Used to force unbind target render SRV (needs to be implemented in the PPMat as some might use the depth SRV as well)
	
	// Generate Mips
	gameContext.pDeviceContext->GenerateMips(m_pRenderTarget->GetShaderResourceView());
}

void PostProcessingMaterial::CreateVertexBuffer(const GameContext& gameContext)
{
	m_NumVertices = 4;
	m_VertexBufferStride = sizeof(VertexPosTex);

	//TODO: complete
	//Create vertex array containing three elements in system memory
	
	VertexPosTex vertices[]
	{
		VertexPosTex{DirectX::XMFLOAT3(-1.f, 1.f, 0.f), DirectX::XMFLOAT2(0.f, 0.f)}
		, VertexPosTex{DirectX::XMFLOAT3(1.f, 1.f, 0.f), DirectX::XMFLOAT2(1.f, 0.f)}
		, VertexPosTex{DirectX::XMFLOAT3(1.f, -1.f, 0.f), DirectX::XMFLOAT2(1.f, 1.f)}
		, VertexPosTex{DirectX::XMFLOAT3(-1.f, -1.f, 0.f), DirectX::XMFLOAT2(0.f, 1.f)}
	};
	
	//fill a buffer description to copy the vertexdata into graphics memory
	D3D11_BUFFER_DESC bdesc{};
	bdesc.CPUAccessFlags = 0;
	bdesc.Usage = D3D11_USAGE_DEFAULT;
	bdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bdesc.MiscFlags = 0;
	bdesc.ByteWidth = m_VertexBufferStride * m_NumVertices;

	D3D11_SUBRESOURCE_DATA subresource;
	subresource.pSysMem = vertices;
	//create a ID3D10Buffer in graphics memory containing the vertex info
	const auto hr{ gameContext.pDevice->CreateBuffer(&bdesc, &subresource, &m_pVertexBuffer) };
	Logger::LogHResult(hr, L"PostProcessingMaterial::CreateVertexBuffer > Could not create Vertex Buffer!");
}

void PostProcessingMaterial::CreateInputBuffer(const GameContext& gameContext)
{
	m_NumIndices = 6;

	D3D11_BUFFER_DESC bdesc{};
	bdesc.CPUAccessFlags = 0;
	bdesc.Usage = D3D11_USAGE_IMMUTABLE;
	bdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bdesc.MiscFlags = 0;
	bdesc.ByteWidth = sizeof(UINT) * m_NumIndices;

	UINT indices[]{ 0, 1, 3, 1, 2, 3 };
	D3D11_SUBRESOURCE_DATA subresource;
	subresource.pSysMem = indices;
	const auto hr{ gameContext.pDevice->CreateBuffer(&bdesc, &subresource, &m_pIndexBuffer) };
	Logger::LogHResult(hr, L"PostProcessingMaterial::CreateVertexBuffer > Could not create Vertex Buffer!");
}