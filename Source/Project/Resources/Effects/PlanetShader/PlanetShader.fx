//DX11 - FLAT SHADER
//Digital Arts & Entertainment
//Firstname: Thomas
//Lastname: Vincent
//Class: 2DAE02

#include "../Include/Lib.fx"

//GLOBAL VARIABLES
//****************
float4x4 gWorld : World;
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldViewProj_Light;
float3 gLightDirection;
float gMaxHeight;
float gMinHeight;
float gShadowIllumination;

cbuffer ColorSettings
{
	float3 gUnderwaterGroundColor;
	float3 gShoreColor;
	float3 gPlaneColorLow;
	float3 gPlaneColorHigh;
	float3 gHeightsColor;
	float3 gShoreColorSettings;
	float3 gPlaneColorSettings;
};

Texture2D gShadowMap;

struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Height : TEXCOORD;
};

struct VS_Output
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Height : TEXCOORD;
	float4 LPos : TEXCOORD1;
};


//STATES
//******

SamplerComparisonState cmpSampler
{
	// sampler state
	Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
	AddressU = MIRROR;
	AddressV = MIRROR;

	// sampler comparison state
	ComparisonFunc = LESS_EQUAL;
};

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

//MAIN VERTEX SHADER
//******************
VS_Output MainVS(VS_INPUT input)
{
	VS_Output output = (VS_Output)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.Normal = mul(input.Normal, (float3x3)gWorld);
	output.Height = input.Height;
	output.LPos = mul(float4(input.Position, 1.0f), gWorldViewProj_Light);

	return output;
}

//MAIN PIXEL SHADER
//*****************
float4 MainPS(VS_Output input) : SV_TARGET
{
	float shadowValue = EvaluateShadowMap(gShadowMap, cmpSampler, input.LPos, 5.f, 0.01f);

	//HalfLambert Diffuse :)
	float diffuseStrength = dot(input.Normal, -gLightDirection);
	diffuseStrength = diffuseStrength * 0.5 + 0.5;
	diffuseStrength = saturate(diffuseStrength);

	float3 color;

	float height = remap01(input.Height.x, 0.f, gMaxHeight);

	if (height == 0.f)
	{
		color = gUnderwaterGroundColor;
	}
	else
	{
		float maxGrassHeight1 = saturate(gPlaneColorSettings.x + gPlaneColorSettings.y);
		float maxGrassHeight2 = saturate(gPlaneColorSettings.x + gPlaneColorSettings.z);
		float maxShoreHeight1 = saturate(gShoreColorSettings.x + gShoreColorSettings.y);
		float maxShoreHeight2 = saturate(gShoreColorSettings.x + gShoreColorSettings.z);

		color = lerp(gPlaneColorLow, gPlaneColorHigh, remap01(height, 0.f, maxGrassHeight2));
		color = lerp(gShoreColor, color, remap01(height, maxShoreHeight1, maxShoreHeight2));

		color = lerp(color, gHeightsColor, remap01(height, maxGrassHeight1, maxGrassHeight2));
		color = lerp(color, gHeightsColor, remap01(input.Height.y, 0.f, 0.2f));
	}

	float3 color_rgb = color * diffuseStrength;

	shadowValue = saturate(shadowValue + gShadowIllumination); //Avoid pitch black shadows
	return float4(color_rgb * shadowValue, 1.f);
}


//TECHNIQUES
//**********
technique11 DefaultTechnique {
	pass p0 {
		SetRasterizerState(BackCulling);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

		SetVertexShader(CompileShader(vs_5_0, MainVS()));
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, MainPS()));
	}
}