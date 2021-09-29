#pragma once
#include "HelperStruct.h"
#include "Singleton.h"

class PlanetComponent;
class PlanetGenMaterial;

struct PlanetGenDescription
{
	NoiseParameter BaseShapeParams = {};
	NoiseParameter DetailParams = {};
	NoiseParameter MountainParams = {};
	NoiseParameter MountainMaskParams = {};
	ColorSettings ColorSettings = {};
	DirectX::XMFLOAT3 OceanColorShallow = {};
	DirectX::XMFLOAT3 OceanColorDeep = {};
	DirectX::XMFLOAT3 RotationAxis = {};
	float RotationSpeed = {};

	int Subdivision = 1;
	int Scaling = 1;

	float OceanDepth = 0.f;
	float OceanDepthMultiplier = 1.f;

	UINT ResourceCount = 0;
};

class PlanetGenerator : public Singleton<PlanetGenerator>
{
public:
	
	PlanetGenerator(const PlanetGenerator& other) = delete;
	PlanetGenerator(PlanetGenerator&& other) noexcept = delete;
	PlanetGenerator& operator=(const PlanetGenerator& other) = delete;
	PlanetGenerator& operator=(PlanetGenerator&& other) noexcept = delete;

	void Init(ID3D11Device* pDevice);
	void Generate(const GameContext& gameContext, PlanetComponent* pPlanetComp, bool generateCollider = false, bool spawnResources = false) const;
	void GenerateCollider(PlanetComponent* pPlanetComp) const;
	void GenerateResource(PlanetComponent* pPlanetComp) const;

	static PlanetGenDescription GenerateRandomPlanetDesc();

	static const int MAXVERTEXOUTPUT = 368640;

private:
	friend class Singleton<PlanetGenerator>;

	explicit PlanetGenerator();
	~PlanetGenerator();

	MeshFilter* m_pMeshFilter;
	PlanetGenMaterial* m_pPlanetMaterial;
	
	bool m_Initialised;

	void GetVertices(const GameContext& gameContext, PlanetComponent* pPlanetComp) const;
};

