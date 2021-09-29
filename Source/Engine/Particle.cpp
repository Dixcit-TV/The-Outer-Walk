#include "stdafx.h"
#include "Particle.h"

// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
using namespace DirectX;

Particle::Particle(const ParticleEmitterSettings& emitterSettings):
	m_VertexInfo(ParticleVertex()),
	m_EmitterSettings(emitterSettings),
	m_IsActive(false),
	m_TotalEnergy(0),
	m_CurrentEnergy(0),
	m_SizeGrow(0),
	m_InitSize(0)
{}

void Particle::Update(const GameContext& gameContext, const XMFLOAT4& rotationQuat)
{
	if (!m_IsActive)
		return;
	
	const float elpased{ gameContext.pGameTime->GetElapsed() };
	m_CurrentEnergy -= elpased;
	if (m_CurrentEnergy <= 0)
	{
		m_IsActive = false;
		return;
	}

	const float lifePercent{ m_CurrentEnergy / m_TotalEnergy };
	XMStoreFloat3(&m_VertexInfo.Position, XMLoadFloat3(&m_VertexInfo.Position) 
		+ XMVector3Rotate(XMLoadFloat3(&m_EmitterSettings.Velocity), XMLoadFloat4(&rotationQuat)) * elpased);
	m_VertexInfo.Color = m_EmitterSettings.Color;
	m_VertexInfo.Color.w = lifePercent * 2;
	m_VertexInfo.Rotation += m_EmitterSettings.AngularVelocity * elpased;
	
	m_VertexInfo.Size = m_InitSize * (1 + (m_SizeGrow - 1) * (1 - lifePercent));
}

void Particle::Init(const XMFLOAT3& initPosition)
{
	m_IsActive = true;
	m_TotalEnergy = m_CurrentEnergy = randF(m_EmitterSettings.MinEnergy, m_EmitterSettings.MaxEnergy);

	const float randYaw{ randF(-XM_PI, XM_PI) };
	const float randPitch{ randF(-XM_PI, XM_PI) };
	const float randRoll{ randF(-XM_PI, XM_PI) };

	XMVECTOR direction{ XMVectorSet(1.f, 0.f, 0.f, 0.f) };
	const XMMATRIX randMat{ XMMatrixRotationRollPitchYaw(randPitch, randYaw, randRoll) };
	direction = XMVector3TransformNormal(direction, randMat);
	
	const float randDistance{ randF(m_EmitterSettings.MinEmitterRange, m_EmitterSettings.MaxEmitterRange) };
	XMStoreFloat3(&m_VertexInfo.Position, XMLoadFloat3(&initPosition) + direction * randDistance);

	m_InitSize = m_VertexInfo.Size = randF(m_EmitterSettings.MinSize, m_EmitterSettings.MaxSize);
	m_SizeGrow = randF(m_EmitterSettings.MinSizeGrow, m_EmitterSettings.MaxSizeGrow);

	m_VertexInfo.Rotation = randF(-XM_PI, XM_PI);
}

void Particle::SetInactive()
{
	m_IsActive = false;
}
