#pragma once
#include "BaseComponent.h"
#include "PlanetGenerator.h"

class PlanetMaterial;

class PlanetComponent : public BaseComponent
{
public:
	PlanetComponent(const PlanetComponent& other) = delete;
	PlanetComponent(PlanetComponent&& other) noexcept = delete;
	PlanetComponent& operator=(const PlanetComponent& other) = delete;
	PlanetComponent& operator=(PlanetComponent&& other) noexcept = delete;
	explicit PlanetComponent(bool random = false);
	explicit PlanetComponent(PlanetGenDescription genDesc);
	virtual ~PlanetComponent() override;

	ID3D11Buffer* const GetVertexBuffer() const { return m_pVertexBuffer; }

	const std::vector<VertexPosNormTex>& GetVertices() const { return m_Vertices; }
	void SetVertices(std::vector<VertexPosNormTex>&& vertices) { m_Vertices = vertices; }

	unsigned int GetTriangleCount() const { return m_TriangleCount; }
	void SetTriangleCount(unsigned int count) { m_TriangleCount = count; }

	float GetGravity() const { return m_Gravity; }

	void SetMaxHeight(float maxHeight) { m_MaxHeight = maxHeight; }
	void SetMinHeight(float minHeight) { m_MinHeight = minHeight; }

	float GetMaxHeight() const { return m_MaxHeight; }
	float GetMinHeight() const { return m_MinHeight; }

	const PlanetGenDescription& GetDesc() const { return m_PlanetGenDesc; }
	PlanetGenDescription& GetDesc() { return m_PlanetGenDesc; }

	DirectX::XMFLOAT4 GetFrameRotation(float deltaTime) const;

protected:
	void Update(const GameContext& gameContext) override;
	void Draw(const GameContext& gameContext) override;
	void Initialize(const GameContext& gameContext) override;
	void DrawShadowMap(const GameContext& gameContext) override;

private:
	PlanetGenDescription m_PlanetGenDesc;
	ID3D11Buffer* m_pVertexBuffer;

	PlanetMaterial* m_pMaterial = nullptr;

	std::vector<VertexPosNormTex> m_Vertices;
	unsigned int m_TriangleCount;

	float m_MaxHeight;
	float m_MinHeight;
	float m_Gravity;
	
	bool m_MaterialSet = false;

	bool m_CastShadows = true;
	bool m_IsInitialized;

	static PlanetGenDescription InitGenDesc(bool random);
};

