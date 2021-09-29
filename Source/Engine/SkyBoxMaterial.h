#pragma once
#include "Material.h"

class TextureData;

class SkyBoxMaterial : public Material
{
public:
	explicit SkyBoxMaterial();
	virtual ~SkyBoxMaterial() override = default;

	SkyBoxMaterial(const SkyBoxMaterial&) = delete;
	SkyBoxMaterial& operator=(const SkyBoxMaterial&) = delete;
	SkyBoxMaterial(SkyBoxMaterial&&) noexcept = delete;
	SkyBoxMaterial& operator=(SkyBoxMaterial&&) noexcept = delete;

	void SetCubeMap(const std::wstring& assetFile);

protected:
	void LoadEffectVariables() override;
	void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent) override;

private:
	TextureData* m_pCubeMapTexture;
	ID3DX11EffectShaderResourceVariable* m_pCubeMapSRVariable;
};

