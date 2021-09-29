#pragma once
#include "GameObject.h"
class ModelComponent;

class SkyBox : public GameObject
{
public:
	explicit SkyBox(const std::wstring& cubeMapPath);

	SkyBox(const SkyBox& other) = delete;
	SkyBox(SkyBox&& other) noexcept = delete;
	SkyBox& operator=(const SkyBox& other) = delete;
	SkyBox& operator=(SkyBox&& other) noexcept = delete;
	virtual ~SkyBox() = default;

protected:
	void Initialize(const GameContext & gameContext) override;

	std::wstring m_CubeMapPath;
};

