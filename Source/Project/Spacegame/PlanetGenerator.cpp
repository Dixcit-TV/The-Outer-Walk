#include "stdafx.h"
#include "PlanetGenerator.h"

#include "ContentManager.h"
#include "MeshFilter.h"
#include "PlanetComponent.h"
#include "TransformComponent.h"
#include "../../Materials/PlanetMaterials/PlanetGenMaterial.h"
#include <RigidBodyComponent.h>


#include "ColliderComponent.h"
#include "GameObject.h"
#include "PhysxManager.h"
#include <numeric>

#include "OrePrefab.h"

PlanetGenerator::PlanetGenerator()
	: m_pMeshFilter(nullptr)
	, m_pPlanetMaterial(nullptr)
	, m_Initialised(false)
{}

PlanetGenerator::~PlanetGenerator()
{
	SafeDelete(m_pPlanetMaterial);
}

void PlanetGenerator::Init(ID3D11Device* pDevice)
{
	if (m_Initialised)
		return;

	GameContext gc{};
	gc.pDevice = pDevice;
	
	m_pMeshFilter = ContentManager::Load<MeshFilter>(L"Resources/Meshes/icosahedron.ovm");
	m_pMeshFilter->BuildIndexBuffer(gc);

	m_pPlanetMaterial = new PlanetGenMaterial();
	m_pPlanetMaterial->Initialize(pDevice);

	m_pMeshFilter->BuildVertexBuffer(gc, m_pPlanetMaterial->m_InputLayoutId, m_pPlanetMaterial->m_InputLayoutSize, m_pPlanetMaterial->m_InputLayoutDescription);

	m_Initialised = true;
}

void PlanetGenerator::Generate(const GameContext& gameContext, PlanetComponent* pPlanetComp, bool generateCollider, bool spawnResources) const
{
	auto* pDeviceContext{ gameContext.pDeviceContext };
	
	D3D11_QUERY_DESC queryDesc{};
	queryDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
	ID3D11Query* pQuery;
	auto hr = gameContext.pDevice->CreateQuery(&queryDesc, &pQuery);
	bool queryStarted = false;
	
	if(!Logger::LogHResult(hr, L"PlanetGenerator::Generate > Could not create query!"))
	{
		pDeviceContext->Begin(pQuery);
		queryStarted = true;
	}
	
	const DirectX::XMFLOAT4X4& worldMatrix{ pPlanetComp->GetTransform()->GetWorld() };
	DirectX::XMFLOAT4X4 wvpMatrix{ };
	const auto world = XMLoadFloat4x4(&worldMatrix);
	const auto viewProjection = XMLoadFloat4x4(&gameContext.pCamera->GetViewProjection());	
	XMStoreFloat4x4(&wvpMatrix, world * viewProjection);

	const PlanetGenDescription& genDesc{ pPlanetComp->GetDesc() };
	PlanetGenMaterial::SetWorld(worldMatrix);
	PlanetGenMaterial::SetWorldViewProjection(wvpMatrix);
	PlanetGenMaterial::SetSubdivision(genDesc.Subdivision);
	PlanetGenMaterial::SetScale(genDesc.Scaling);
	PlanetGenMaterial::SetBaseShapeParams(genDesc.BaseShapeParams);
	PlanetGenMaterial::SetDetailParams(genDesc.DetailParams);
	PlanetGenMaterial::SetMountainParams(genDesc.MountainParams);
	PlanetGenMaterial::SetMountainMaskParams(genDesc.MountainMaskParams);
	PlanetGenMaterial::SetOceanDepth(genDesc.OceanDepth);
	PlanetGenMaterial::SetOceanDepthMultiplier(genDesc.OceanDepthMultiplier);

	VertexBufferData vBufferData{ m_pMeshFilter->GetVertexBufferData(gameContext, m_pPlanetMaterial->m_InputLayoutId) };
	UINT stride{ vBufferData.VertexStride };
	UINT offset{ 0 };
	pDeviceContext->IASetVertexBuffers(0, 1, &vBufferData.pVertexBuffer, &stride, &offset);
	pDeviceContext->IASetInputLayout(m_pPlanetMaterial->m_pInputLayout);
	pDeviceContext->IASetIndexBuffer(m_pMeshFilter->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	ID3D11Buffer* const soBuffer{ pPlanetComp->GetVertexBuffer() };
	pDeviceContext->SOSetTargets(1, &soBuffer, &offset);

	//TODO: invoke draw call
	D3DX11_TECHNIQUE_DESC techDesc;
	auto* pTechnique{ m_pPlanetMaterial->m_pGenTech };
	pTechnique->GetDesc(&techDesc);
	for (UINT p{ 0 }; p < techDesc.Passes; ++p)
	{
		pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_pMeshFilter->m_IndexCount, 0, 0);
	}

	ID3D11Buffer* bufferArray[1] = { nullptr };
	pDeviceContext->SOSetTargets(1, bufferArray, &offset);

	if (queryStarted)
	{
		gameContext.pDeviceContext->End(pQuery);
		D3D11_QUERY_DATA_PIPELINE_STATISTICS  queryData{};
		
		while (S_OK != pDeviceContext->GetData(pQuery, &queryData, sizeof(D3D11_QUERY_DATA_PIPELINE_STATISTICS), 0));

		pPlanetComp->SetTriangleCount(static_cast<unsigned>(queryData.GSPrimitives));

		SafeRelease(pQuery);
	}

	GetVertices(gameContext, pPlanetComp);

	if (generateCollider)
		GenerateCollider(pPlanetComp);

	if (spawnResources)
		GenerateResource(pPlanetComp);
}

void PlanetGenerator::GenerateCollider(PlanetComponent* pPlanetComp) const
{
	GameObject* pPlanet{ pPlanetComp->GetGameObject() };
	auto* pCollider = pPlanet->GetComponent<ColliderComponent>();
	if (!pCollider)
		return;

	const std::vector<VertexPosNormTex>& vertices{ pPlanetComp->GetVertices() };
	const size_t vCount{ vertices.size() };

	std::vector<physx::PxU32> indices{  };
	indices.resize(vCount);
	std::iota(std::begin(indices), std::end(indices), 0);

	auto* pCooking = PhysxManager::GetInstance()->GetCooking();
	physx::PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = vCount;
	meshDesc.points.stride = sizeof(VertexPosNormTex);
	meshDesc.points.data = std::data(vertices);

	meshDesc.triangles.count = pPlanetComp->GetTriangleCount();
	meshDesc.triangles.stride = 3 * sizeof(physx::PxU32);
	meshDesc.triangles.data = std::data(indices);

	physx::PxTriangleMesh* pTriangleMesh = pCooking->createTriangleMesh(meshDesc, PhysxManager::GetInstance()->GetPhysics()->getPhysicsInsertionCallback());
	std::shared_ptr<physx::PxGeometry> pGeo{ std::make_shared<physx::PxTriangleMeshGeometry>(pTriangleMesh) };

	pCollider->UpdateGeometry(pGeo);
}

void PlanetGenerator::GenerateResource(PlanetComponent* pPlanetComp) const
{
	using namespace DirectX;
	
	GameObject* pPlanet{ pPlanetComp->GetGameObject() };
	std::vector<GameObject*> childResources{ pPlanet->GetChildrenWithTag(L"Resource") };
	std::for_each(std::begin(childResources), std::end(childResources), [pPlanet](GameObject* pChild) { pPlanet->RemoveChild(pChild); });

	const std::vector<VertexPosNormTex>& vertices{ pPlanetComp->GetVertices() };

	const UINT resourceCount{ pPlanetComp->GetDesc().ResourceCount };
	const UINT interval{ vertices.size() / resourceCount };
	UINT count = 0;
	UINT childCount = 0;

	for (const VertexPosNormTex& v : vertices)
	{
		if (childCount < static_cast<size_t>(resourceCount)
			&& v.TexCoord.y > 0.2f
			&& v.TexCoord.x > 0.f)
		{
			if (count > interval)
			{
				++childCount;
				const XMVECTOR posV{ XMLoadFloat3(&v.Position) };
				const XMMATRIX view = XMMatrixLookAtLH(posV, posV * 1.1f, XMVectorSet(0.f, 1.f, 0.f, 0.f));
				XMFLOAT4 quat{};
				XMStoreFloat4(&quat, XMQuaternionRotationMatrix(view));

				OrePrefab* pOre{ new OrePrefab(v.Position, quat) };
				pOre->SetTag(L"Resource");
				pPlanet->AddChild(pOre);
				count = 0;
			}
			else
				++count;
		}
	}
}

PlanetGenDescription PlanetGenerator::GenerateRandomPlanetDesc()
{	
	PlanetGenDescription desc{};
	const float minScale{ 150.f };
	const float middleScale{ 175.f };
	const float maxScale{ 200.f };
	const float scaleDiff{ maxScale - minScale };

	int r = randI(1, 3);
	desc.Scaling = r > 1 ? static_cast<int>(randF(middleScale, maxScale)) : static_cast<int>(randF(minScale, maxScale));
	const float scaleRatio{ (desc.Scaling - minScale) / scaleDiff };
	
	desc.Subdivision = static_cast<int>(64.f * desc.Scaling / maxScale);

	const float baseShapeMaxHeight{ 4.f };
	const float baseShapeMinHeight{ 1.5f };
	desc.BaseShapeParams = { randF(baseShapeMinHeight, baseShapeMaxHeight), randF(4.f, 6.f), randF(0.75f, 2.5f), randF(2.f, 4.f), randF(0.2f, 0.35f), randF(0.f, 5.f), 0.f };
	const float baseShapeRatio{ (desc.BaseShapeParams.data.NoiseHeight - baseShapeMinHeight) / (baseShapeMaxHeight - baseShapeMinHeight) };
	desc.DetailParams = { randF(2.f, 6.f), randF(3.f, 4.f), randF(1.5f, 4.f), randF(3.f, 7.f), randF(0.3f, 0.6f), randF(0.f, 5.f), randF(-1.f, 4.f) };
	desc.MountainParams = { randF(2.f, 4.f) + 3.f * (1 - baseShapeRatio), randF(3.f, 4.f), randF(1.f, 2.f), randF(3.f, 5.f), randF(0.3f, 0.5f), randF(0.f, 5.f), 0.f };
	desc.MountainMaskParams = {};
	desc.OceanDepthMultiplier = randF(5.f, 10.f) + 5.f * (1 - baseShapeRatio);
	desc.OceanDepth = desc.OceanDepthMultiplier * (1 - randF(0.4f, 0.6f));
	desc.ColorSettings.UnderwaterGroundColor = DirectX::XMFLOAT3{ 0.368f, 0.298f, 0.211f };
	desc.ColorSettings.ShoreColor = DirectX::XMFLOAT3{ 0.6f, 0.55f, 0.35f };
	desc.ColorSettings.PlaneColorLow = DirectX::XMFLOAT3{ 0.3f, 0.5f, 0.25f };
	desc.ColorSettings.PlaneColorHigh = DirectX::XMFLOAT3{ 0.20f, 0.325f, 0.20f };
	desc.ColorSettings.HeightsColor = DirectX::XMFLOAT3{ 0.368f, 0.298f, 0.211f };
	desc.ColorSettings.ShoreColorSettings = DirectX::XMFLOAT3{ 0.01f, -0.f, 0.01f };
	desc.ColorSettings.PlaneColorSettings = DirectX::XMFLOAT3{ 0.4f, -0.1f, 0.1f };
	desc.OceanColorShallow = DirectX::XMFLOAT3{ 0.345f, 0.929f, 0.921f };
	desc.OceanColorDeep = DirectX::XMFLOAT3{ 0.066f, 0.207f, 0.451f };

	float x{ randF(-1.f, 1.f) };
	float y{ randF(-1.f, 1.f) };
	float z{ randF(-1.f, 1.f) };
	XMStoreFloat3(&desc.RotationAxis, DirectX::XMVector3Normalize(DirectX::XMVectorSet(x, y, z, 0.f)));
	desc.RotationSpeed = DirectX::XMConvertToRadians(randF(0.25f, 1.25f)) * (randI(0, 1) > 0 ? -1 : 1);
	desc.ResourceCount = randI(50, 150) + static_cast<UINT>(50 * scaleRatio);

	return desc;
}

void PlanetGenerator::GetVertices(const GameContext& gameContext, PlanetComponent* pPlanetComp) const
{
	auto* pDeviceContext{ gameContext.pDeviceContext };
	
	ID3D11Buffer* pPlanetBuffer{ pPlanetComp->GetVertexBuffer() };
	//Get Vertices
	D3D11_BUFFER_DESC sbdesc{};
	pPlanetBuffer->GetDesc(&sbdesc);
	sbdesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	sbdesc.Usage = D3D11_USAGE_STAGING;
	sbdesc.BindFlags = 0;
	sbdesc.MiscFlags = 0;

	ID3D11Buffer* pStagingVBuffer;
	auto hr = gameContext.pDevice->CreateBuffer(&sbdesc, NULL, &pStagingVBuffer);
	if (Logger::LogHResult(hr, L"PlanetGenerator::Generate > Staging Vertex buffer creation failed!"))
		return;

	pDeviceContext->CopyResource(pStagingVBuffer, pPlanetBuffer);

	const unsigned triangleCount{ pPlanetComp->GetTriangleCount() };
	const size_t vCount = static_cast<size_t>(triangleCount * 3);

	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	hr = pDeviceContext->Map(pStagingVBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
	if (Logger::LogHResult(hr, L"PlanetGenerator::Generate > Could not map staging vertex buffer!"))
		return;

	std::vector<VertexPosNormTex> verts{  };
	verts.resize(vCount);
	memcpy(std::data(verts), mappedResource.pData, vCount * sizeof(VertexPosNormTex));
	pDeviceContext->Unmap(pStagingVBuffer, 0);
	SafeRelease(pStagingVBuffer);

	float maxHeight = -FLT_MAX;
	float minHeight = FLT_MAX;

	for (const VertexPosNormTex& v : verts)
	{
		const float height{ v.TexCoord.x };
		if (maxHeight < height)
			maxHeight = height;

		if (minHeight > height)
			minHeight = height;
	}

	pPlanetComp->SetMaxHeight(maxHeight);
	pPlanetComp->SetMinHeight(minHeight);
	pPlanetComp->SetVertices(std::move(verts));
}