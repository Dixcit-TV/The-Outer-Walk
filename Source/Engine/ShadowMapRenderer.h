#pragma once

class MeshFilter;
class RenderTarget;
class ShadowMapMaterial;
class ShadowMapMaterial_Skinned;

class ShadowMapRenderer final
{
public:
	ShadowMapRenderer() = default;
	~ShadowMapRenderer();

	ShadowMapRenderer(const ShadowMapRenderer& other) = delete;
	ShadowMapRenderer(ShadowMapRenderer&& other) noexcept = delete;
	ShadowMapRenderer& operator=(const ShadowMapRenderer& other) = delete;
	ShadowMapRenderer& operator=(ShadowMapRenderer&& other) noexcept = delete;

	void Initialize(const GameContext& gameContext);

	void UpdateLightViewSettings(float size, float nearPlane = -1.f, float farPlane = -1.f);

	void SetShadowIllumination(float illumination) { m_ShadowIllumination = illumination; }
	float GetShadowIllumination() const { return m_ShadowIllumination; }
	void SetLight(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction);
	const DirectX::XMFLOAT3& GetLightDirection() const { return m_LightDirection; }
	const DirectX::XMFLOAT4X4& GetLightVP() const { return m_LightVP; }

	void Begin(const GameContext& gameContext) const;
	void End(const GameContext& gameContext) const;

	void Draw(const GameContext& gameContext, MeshFilter* pMeshFilter, const DirectX::XMFLOAT4X4& world, const std::vector<DirectX::XMFLOAT4X4>& bones = std::vector<DirectX::XMFLOAT4X4>()) const;
	void DrawAuto(const GameContext& gameContext, ID3D11Buffer* pVBuffer, UINT stride, const DirectX::XMFLOAT4X4& world, const std::vector<DirectX::XMFLOAT4X4>& bones = std::vector<DirectX::XMFLOAT4X4>()) const;
	void UpdateMeshFilter(const GameContext& gameContext, MeshFilter* pMeshFilter) const;
	ShadowMapMaterial* GetMaterial() const { return m_pShadowMat; }
	ID3D11ShaderResourceView* GetShadowMap() const;

private:
	//LIGHT
	DirectX::XMFLOAT3 m_LightPosition = {}, m_LightDirection = {};
	DirectX::XMFLOAT4X4 m_LightVP = {};
	float m_Size = 100.0f;
	float m_NearPlane = 0.01f;
	float m_FarPlane = 500.f;
	float m_ShadowIllumination = 0.f;

	ShadowMapMaterial* m_pShadowMat = nullptr;
	RenderTarget* m_pShadowRT = nullptr;
	bool m_IsInitialized = false;
};

