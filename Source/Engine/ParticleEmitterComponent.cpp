#include "stdafx.h"
#include "ParticleEmitterComponent.h"
 #include <utility>
#include "EffectHelper.h"
#include "ContentManager.h"
#include "TextureDataLoader.h"
#include "Particle.h"
#include "TransformComponent.h"

ParticleEmitterComponent::ParticleEmitterComponent(std::wstring&& assetFile, bool playOnStart, int particleCount, bool isBillboard):
	m_pVertexBuffer(nullptr),
	m_pEffect(nullptr),
	m_pBillboardingVariable(nullptr),
	m_pParticleTexture(nullptr),
	m_pInputLayout(nullptr),
	m_pInputLayoutSize(0),
	m_Particles(particleCount),
	m_Settings(ParticleEmitterSettings()),
	m_ParticleCount(particleCount),
	m_ActiveParticles(0),
	m_LastParticleInit(0.0f),
	m_AssetFile(std::move(assetFile)),
	m_IsBillboard(isBillboard),
	m_IsPlaying(playOnStart)
{
	std::generate(std::begin(m_Particles), std::end(m_Particles), [&setting = m_Settings] () { return new Particle(setting); });
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	std::for_each(std::begin(m_Particles), std::end(m_Particles), [](Particle* p) { delete p; });
	m_Particles.clear();

	SafeRelease(m_pInputLayout);
	SafeRelease(m_pVertexBuffer);
}

void ParticleEmitterComponent::Play()
{
	m_IsPlaying = true;
}

void ParticleEmitterComponent::Stop(bool clear)
{
	m_IsPlaying = false;

	if (clear)
		Clear();
}

void ParticleEmitterComponent::Clear()
{
	std::for_each(std::begin(m_Particles), std::end(m_Particles), [](Particle* p) { p->SetInactive(); });
}

void ParticleEmitterComponent::Initialize(const GameContext& gameContext)
{
	LoadEffect(gameContext);
	CreateVertexBuffer(gameContext);
	m_pParticleTexture = ContentManager::Load<TextureData>(m_AssetFile);
}

void ParticleEmitterComponent::LoadEffect(const GameContext& gameContext)
{
	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"Resources/Effects/ParticleRenderer.fx");
	m_pDefaultTechnique = m_pEffect->GetTechniqueByIndex(0);

	if (!m_pWvpVariable)
	{
		m_pWvpVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pWvpVariable->IsValid())
		{
			Logger::LogWarning(L"ParticleEmitterComponent::LoadEffect() > \'gWorldViewProj\' variable not found!");
			m_pWvpVariable = nullptr;
		}
	}

	if (!m_pViewInverseVariable)
	{
		m_pViewInverseVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
		if (!m_pViewInverseVariable->IsValid())
		{
			Logger::LogWarning(L"ParticleEmitterComponent::LoadEffect() > \'gViewInverse\' variable not found!");
			m_pViewInverseVariable = nullptr;
		}
	}

	if (!m_pBillboardingVariable)
	{
		m_pBillboardingVariable = m_pEffect->GetVariableByName("gIsBillboard")->AsScalar();
		if (!m_pBillboardingVariable->IsValid())
		{
			Logger::LogWarning(L"ParticleEmitterComponent::LoadEffect() > \'gIsBillboard\' variable not found!");
			m_pBillboardingVariable = nullptr;
		}
	}

	if (!m_pTextureVariable)
	{
		m_pTextureVariable = m_pEffect->GetVariableByName("gParticleTexture")->AsShaderResource();
		if (!m_pTextureVariable->IsValid())
		{
			Logger::LogWarning(L"ParticleEmitterComponent::LoadEffect() > \'gParticleTexture\' variable not found!");
			m_pTextureVariable = nullptr;
		}
	}

	EffectHelper::BuildInputLayout(gameContext.pDevice, m_pDefaultTechnique, &m_pInputLayout, m_pInputLayoutSize);
}

void ParticleEmitterComponent::CreateVertexBuffer(const GameContext& gameContext)
{
	if (m_pVertexBuffer)
		SafeRelease(m_pVertexBuffer);

	D3D11_BUFFER_DESC bDesc{};
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bDesc.Usage = D3D11_USAGE_DYNAMIC;
	bDesc.ByteWidth = sizeof(ParticleVertex) * m_ParticleCount;
	bDesc.MiscFlags = 0;

	const auto hr{ gameContext.pDevice->CreateBuffer(&bDesc, nullptr, &m_pVertexBuffer) };
	Logger::LogHResult(hr, L"ParticleEmitterComponent::CreateVertexBuffer > Failed to create vertex buffer!");
}

void ParticleEmitterComponent::Update(const GameContext& gameContext)
{
	if (m_ActiveParticles == 0 && !m_IsPlaying)
		return;
	
	const float particleInterval{ (m_Settings.MaxEnergy + m_Settings.MinEnergy) * 0.5f / static_cast<float>(m_ParticleCount) };
	m_LastParticleInit += gameContext.pGameTime->GetElapsed();
	m_ActiveParticles = 0;

	//BUFFER MAPPING CODE [PARTIAL :)]
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	const auto hr = gameContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (Logger::LogHResult(hr, L"ParticleEmitterComponent::Update > Could not map dynamic vertex buffer!"))
		return;

	ParticleVertex* pBuffer{ static_cast<ParticleVertex*>(mappedResource.pData) };
	const DirectX::XMFLOAT4& rotQuat{ GetTransform()->GetWorldRotation() };

	for (Particle* p : m_Particles)
	{
		p->Update(gameContext, rotQuat);
		if (p->IsActive())
		{
			pBuffer[m_ActiveParticles] = p->GetVertexInfo();
			++m_ActiveParticles;
		}
		else if (m_LastParticleInit >= particleInterval && m_IsPlaying)
		{
			m_LastParticleInit -= particleInterval;
			p->Init(m_IsBillboard ? GetTransform()->GetWorldPosition() : DirectX::XMFLOAT3{});
			pBuffer[m_ActiveParticles] = p->GetVertexInfo();
			++m_ActiveParticles;
		}
	}

	gameContext.pDeviceContext->Unmap(m_pVertexBuffer, 0);
}

void ParticleEmitterComponent::Draw(const GameContext& )
{}

void ParticleEmitterComponent::PostDraw(const GameContext& gameContext)
{
	if (m_ActiveParticles == 0)
		return;
	
	if (m_pWvpVariable)
	{
		auto wvp = XMLoadFloat4x4(&gameContext.pCamera->GetViewProjection());

		if (!m_IsBillboard)
		{
			wvp = XMLoadFloat4x4(&GetTransform()->GetWorld()) * wvp;
		}

		m_pWvpVariable->SetMatrix(reinterpret_cast<const float*>(&wvp));
	}
		
	if (m_pViewInverseVariable) m_pViewInverseVariable->SetMatrix(gameContext.pCamera->GetViewInverse().m[0]);
	if (m_pBillboardingVariable) m_pBillboardingVariable->SetBool(m_IsBillboard);
	if (m_pTextureVariable) m_pTextureVariable->SetResource(m_pParticleTexture->GetShaderResourceView());

	gameContext.pDeviceContext->IASetInputLayout(m_pInputLayout);
	UINT stride{ sizeof(ParticleVertex) };
	UINT pOffset[1]{ 0 };
	gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, pOffset);
	gameContext.pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pDefaultTechnique->GetDesc(&techDesc);
	for (uint32_t p{}; p < techDesc.Passes; ++p)
	{
		m_pDefaultTechnique->GetPassByIndex(p)->Apply(0, gameContext.pDeviceContext);
		gameContext.pDeviceContext->Draw(m_ActiveParticles, 0);
	}
}
