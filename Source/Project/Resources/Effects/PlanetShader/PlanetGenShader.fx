//DX10 - FLAT SHADER
//Digital Arts & Entertainment
//Environment_Mapping.fx
//Firstname: Thomas
//Lastname: Vincent
//Class: 2DAE02

#include "../Include/Lib.fx"
#include "../Include/noiseSimplex.fx"

//GLOBAL VARIABLES
//****************
float4x4 gWorld : World;
float4x4 gWorldViewProj : WorldViewProjection;

float gSubdivisions = 20;
float gScaling = 1;
float gOceanDepthMultiplier;
float gOceanDepth;

float gBaseShapeParams[7];
float gDetailParams[7];
float gRidgeParams[7];
float gRidgeMaskParams[7];

RasterizerState BackCulling
{
	CullMode = back;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

BlendState NoBlending
{
	BlendEnable[0] = false;
};

struct VS_INPUT
{
	float3 Position : POSITION;
};

struct PATCH_OUTPUT
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
	float3 Position : POSITION;
};

struct DS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Height : TEXCOORD;
};

//MAIN VERTEX SHADER
//******************
VS_INPUT MainVS(VS_INPUT input)
{
	return input;
}

//Constant HULL function
//******************
PATCH_OUTPUT ConstantHS(InputPatch<VS_INPUT, 3> patch, uint patchID : SV_PrimitiveID)
{
	PATCH_OUTPUT output = (PATCH_OUTPUT)0;
	
	output.EdgeTess[0] = gSubdivisions;
	output.EdgeTess[1] = gSubdivisions;
	output.EdgeTess[2] = gSubdivisions;
	
	output.InsideTess = gSubdivisions;
	
	return output;
}

//MAIN HULL SHADER
//******************
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HS_OUTPUT MainHS(InputPatch<VS_INPUT, 3> patch, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
	HS_OUTPUT output = (HS_OUTPUT)0;
	output.Position = patch[i].Position;
	
	return output;
}

float FractalNoise(float3 p, float params[7])
{
	float scale = params[0];
	float layerCount = params[1];		
	float frequency = params[2];
	float freqMultiplier = params[3];
	float ampMultiplier = params[4];
	float offset = params[5];
	float vShift = params[6];

	float amplitude = 1;
	float n = 0;
	for (int idx = 0; idx < layerCount; ++idx)
	{
		float noise = snoise(p * frequency + offset);
		n += noise * amplitude;

		frequency *= freqMultiplier;
		amplitude *= ampMultiplier;
	}

	float height = n * scale;
	return height + vShift;
}


float RidgeNoise(float3 p, float params[7])
{
	float scale = params[0];
	float layerCount = params[1];
	float frequency = params[2];
	float freqMultiplier = params[3];
	float ampMultiplier = params[4];
	float offset = params[5];
	float vShift = params[6];

	//float weight = 1;
	float amplitude = 1;
	float n = 0;
	for (int idx = 0; idx < layerCount; ++idx)
	{
		float noise = 2 * (0.5f - abs(0.5f - snoise(p * frequency + offset)));
		n += noise * amplitude;

		frequency *= freqMultiplier;
		amplitude *= ampMultiplier;
	}

	float height = n * scale * 5.f;
	return max(0, height + vShift);
}

float SmoothedRigdedNoise(float3 p, float params[7])
{
	float nSize = 3; 
	float sum = 0;
	float nBound = (nSize - 1) * 0.5f;
	float x, y;

	float3 normal = normalize(p);
	float3 axisA = cross(normal, float3(0, 1, 0));
	float3 axisB = cross(normal, axisA);

	for (y = -nBound; y <= nBound; y += 1.0)
	{
		for (x = -nBound; x <= nBound; x += 1.0)
		{
			sum += RidgeNoise(p + axisA * x * 0.1 + axisB * y * 0.1, params);
		}
	}

	return sum / (nSize * nSize);
}

//MAIN DOMAIN SHADER
//******************
[domain("tri")]
DS_OUTPUT MainDS(PATCH_OUTPUT patchInput, float3 uvw : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 3> tri)
{
	DS_OUTPUT output = (DS_OUTPUT)0;
	float3 position = tri[0].Position * uvw.x + tri[1].Position * uvw.y + tri[2].Position * uvw.z;
	float3 posDir = normalize(position);

	float n = FractalNoise(position, gBaseShapeParams);
	n = smoothMax(n, -gOceanDepth, 0.34f);

	if (n < 0)
	{
		n *= gOceanDepthMultiplier;
	}
	else
	{
		n += SmoothedRigdedNoise(position, gRidgeParams) * smoothMax(0.f, n, 0.5f);
	}
	n += FractalNoise(position, gDetailParams) * 0.5f;

	output.Height = float2(n, 0.f);

	float3 pos = position + posDir * (n + gScaling);
	output.Position = float4(pos, 1.f);
	output.Normal = posDir;
	return output;
}

[maxvertexcount(3)]
void GS(triangle DS_OUTPUT input[3], inout TriangleStream<DS_OUTPUT> triStream)
{
	float3 baseNormal = (input[0].Normal + input[1].Normal + input[2].Normal) / 3.f;
	float3 normal = normalize(cross(input[2].Position.xyz - input[0].Position.xyz, input[1].Position.xyz - input[0].Position.xyz));
	float slopeAngle = 1 - abs(dot(normal, baseNormal));
	input[0].Normal = normal;
	input[0].Height.y = slopeAngle;
	input[1].Normal = normal;
	input[1].Height.y = slopeAngle;
	input[2].Normal = normal;
	input[2].Height.y = slopeAngle;

	triStream.Append(input[0]);
	triStream.Append(input[2]);
	triStream.Append(input[1]);
}

GeometryShader pGS = CompileShader(gs_5_0, GS());

//TECHNIQUES
//**********
technique11 DefaultTechnique {
	pass p0 {
		SetRasterizerState(BackCulling);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

		SetVertexShader(CompileShader(vs_5_0, MainVS()));
		SetHullShader(CompileShader(hs_5_0, MainHS()));
		SetDomainShader(CompileShader(ds_5_0, MainDS()));
		SetGeometryShader(ConstructGSWithSO(pGS, "SV_POSITION.xyz; NORMAL.xyz; TEXCOORD.xy", NULL, NULL, NULL, -1));
		SetPixelShader(NULL);
	}
}