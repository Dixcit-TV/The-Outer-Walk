#pragma once
#include "PostProcessingMaterial.h"

class PlanetComponent;

class PostOcean : public PostProcessingMaterial
{
public:
	explicit PostOcean(PlanetComponent* pPlanet);
	virtual ~PostOcean() = default;

	PostOcean(const PostOcean& other) = delete;
	PostOcean(PostOcean&& other) noexcept = delete;
	PostOcean& operator=(const PostOcean& other) = delete;
	PostOcean& operator=(PostOcean&& other) noexcept = delete;
protected:
	void LoadEffectVariables() override;
	void UpdateEffectVariables(const GameContext& gameContext, RenderTarget * pRendertarget) override;
	void PostOcean::UnbindEffectVariables(const GameContext& gameContext) override;

private:
	ID3DX11EffectShaderResourceVariable* m_pTextureMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pDepthTextureMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;

	ID3DX11EffectMatrixVariable* m_pProjInverse;
	ID3DX11EffectMatrixVariable* m_pViewInverse;
	ID3DX11EffectMatrixVariable* m_pWorldMatrix;
	ID3DX11EffectScalarVariable* m_pPlanetRadius;
	ID3DX11EffectScalarVariable* m_pTimeVariable;

	ID3DX11EffectVectorVariable* m_pShallowColor;
	ID3DX11EffectVectorVariable* m_pDeepColor;
	ID3DX11EffectScalarVariable* m_pMaxDepth;

	PlanetComponent* m_pPlanet;
};
