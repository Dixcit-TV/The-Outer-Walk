#include "stdafx.h"
#include "ModelAnimator.h"

ModelAnimator::ModelAnimator(MeshFilter* pMeshFilter):
m_pMeshFilter(pMeshFilter),
m_Transforms(std::vector<DirectX::XMFLOAT4X4>()),
m_IsPlaying(false), 
m_Reversed(false),
m_ClipSet(false),
m_TickCount(0),
m_AnimationSpeed(1.0f)
{
	SetAnimation(0);
}

void ModelAnimator::SetAnimation(UINT clipNumber)
{
	//TODO: complete
	//Set m_ClipSet to false
	m_ClipSet = false;
	//Check if clipNumber is smaller than the actual m_AnimationClips vector size
	if (clipNumber >= m_pMeshFilter->m_AnimationClips.size())
	{
		Reset(true);
		Logger::LogFormat(LogLevel::Warning, L"ModelAnimator::SetAnimation ==> Requested clip %i out of range !", clipNumber);
		return;
	}

	SetAnimation(m_pMeshFilter->m_AnimationClips[clipNumber]);
}

void ModelAnimator::SetAnimation(const std::wstring& clipName)
{
	//TODO: complete
	//Set m_ClipSet to false
	m_ClipSet = false;
	//Iterate the m_AnimationClips vector and search for an AnimationClip with the given name (clipName)
	auto clipIt{ std::find_if(std::cbegin(m_pMeshFilter->m_AnimationClips), std::cend(m_pMeshFilter->m_AnimationClips), [&clipName](const AnimationClip& clip)
		{
			return clip.Name == clipName;
		})
	};

	if (clipIt == std::cend(m_pMeshFilter->m_AnimationClips))
	{
		Reset(true);
		Logger::LogFormat(LogLevel::Warning, L"ModelAnimator::SetAnimation ==> Requested clip \"%s\" does not exists !", clipName.c_str());
		return;
	}

	SetAnimation(*clipIt);
}

void ModelAnimator::SetAnimation(const AnimationClip& clip)
{
	//TODO: complete
	//Set m_ClipSet to true
	m_ClipSet = true;
	//Set m_CurrentClip
	m_CurrentClip = clip;
	//Call Reset(false)
	Reset(false);
}

void ModelAnimator::Reset(bool pause)
{
	//TODO: complete
	//If pause is true, set m_IsPlaying to false
	m_IsPlaying = pause ? false : m_IsPlaying;

	//Set m_TickCount to zero
	m_TickCount = 0.f;
	//Set m_AnimationSpeed to 1.0f
	m_AnimationSpeed = 1.f;

	if (m_ClipSet)
	{
		const auto& transforms{ m_CurrentClip.Keys[0].BoneTransforms };
		m_Transforms.assign(std::begin(transforms), std::end(transforms));
	}
	else
	{
		const DirectX::XMFLOAT4X4 identityM{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		m_Transforms.assign(m_CurrentClip.Keys[0].BoneTransforms.size(), identityM);
	}
}

void ModelAnimator::Update(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);
	//TODO: complete
	//We only update the transforms if the animation is running and the clip is set
	if (m_IsPlaying && m_ClipSet)
	{
		//1. 
		//Calculate the passedTicks (see the lab document)
		auto passedTicks = gameContext.pGameTime->GetElapsed() * m_CurrentClip.TicksPerSecond * m_AnimationSpeed;
		//Make sure that the passedTicks stay between the m_CurrentClip.Duration bounds (fmod)
		passedTicks = fmod(passedTicks, m_CurrentClip.Duration);
		//2. 
		//IF m_Reversed is true
		//	Subtract passedTicks from m_TickCount
		//	If m_TickCount is smaller than zero, add m_CurrentClip.Duration to m_TickCount
		//ELSE
		//	Add passedTicks to m_TickCount
		//	if m_TickCount is bigger than the clip duration, subtract the duration from m_TickCount
		if (m_Reversed)
		{
			m_TickCount -= passedTicks;
			if (m_TickCount < 0.f)
				m_TickCount += m_CurrentClip.Duration;
		}
		else
		{
			m_TickCount += passedTicks;
			if (m_TickCount > m_CurrentClip.Duration)
				m_TickCount -= m_CurrentClip.Duration;
		}
		//3.
		//Find the enclosing keys
		AnimationKey keyA, keyB;
		//Iterate all the keys of the clip and find the following keys:
		//keyA > Closest Key with Tick before/smaller than m_TickCount
		//keyB > Closest Key with Tick after/bigger than m_TickCount
		for(const AnimationKey& key : m_CurrentClip.Keys)
		{
			if (key.Tick >= m_TickCount)
			{
				keyB = key;
				break;
			}
		
			keyA = key;
		}

		//4.
		//Interpolate between keys
		//Figure out the BlendFactor (See lab document)
		//Clear the m_Transforms vector
		//FOR every boneTransform in a key (So for every bone)
		//	Retrieve the transform from keyA (transformA)
		//	auto transformA = ...
		// 	Retrieve the transform from keyB (transformB)
		//	auto transformB = ...
		//	Decompose both transforms
		//	Lerp between all the transformations (Position, Scale, Rotation)
		//	Compose a transformation matrix with the lerp-results
		//	Add the resulting matrix to the m_Transforms vector
		const float blendFactor{ (m_TickCount - keyA.Tick) / (keyB.Tick - keyA.Tick) };
		m_Transforms.clear();
		const size_t tCount{ keyA.BoneTransforms.size() };
		for (size_t idx{}; idx < tCount; ++idx)
		{
			DirectX::XMMATRIX transformA{ XMLoadFloat4x4(&keyA.BoneTransforms[idx]) };
			DirectX::XMVECTOR scaleA, posA, quaternionA;

			DirectX::XMMATRIX transformB{ XMLoadFloat4x4(&keyB.BoneTransforms[idx]) };
			DirectX::XMVECTOR scaleB, posB, quaternionB;

			DirectX::XMMatrixDecompose(&scaleA, &quaternionA, &posA, transformA);
			DirectX::XMMatrixDecompose(&scaleB, &quaternionB, &posB, transformB);

			DirectX::XMVECTOR finalScale{ DirectX::XMVectorLerp(scaleA, scaleB, blendFactor) };
			DirectX::XMVECTOR finalPos{ DirectX::XMVectorLerp(posA, posB, blendFactor) };
			DirectX::XMVECTOR finalQuaternion{ DirectX::XMQuaternionSlerp(quaternionA, quaternionB, blendFactor) };

			DirectX::XMMATRIX finalMatrix{ DirectX::XMMatrixAffineTransformation(finalScale, DirectX::XMVECTOR{}, finalQuaternion, finalPos) };
			DirectX::XMFLOAT4X4 transform;
			DirectX::XMStoreFloat4x4(&transform, finalMatrix);

			m_Transforms.push_back(transform);
		}
	}
}
