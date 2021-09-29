/*
******************
* DAE Ubershader *
******************

**This Shader Contains:

- Diffuse (Texture & Color)
	- Regular Diffuse
- Specular
	- Specular Level (Texture & Value)
	- Shininess (Value)
	- Models
		- Blinn
		- Phong
- Ambient (Color)
- EnvironmentMapping (CubeMap)
	- Reflection + Fresnel Falloff
	- Refraction
- Normal (Texture)
- Opacity (Texture & Value)

-Techniques
	- WithAlphaBlending
	- WithoutAlphaBlending
*/

//UberShader.fx
//Firstname: Thomas
//Lastname: Vincent
//Class: 2DAE02

//GLOBAL MATRICES
//***************
// The World View Projection Matrix
float4x4 gMatrixWVP : WORLDVIEWPROJECTION;
// The ViewInverse Matrix - the third row contains the camera position!
float4x4 gMatrixViewInverse : VIEWINVERSE;
// The World Matrix
float4x4 gMatrixWorld : WORLD;

//STATES
//******
RasterizerState gRS_FrontCulling 
{ 
	CullMode = FRONT; 
};

BlendState EnableBlending 
{     
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
};

BlendState DisableBlending
{
	BlendEnable[0] = FALSE;
};

DepthStencilState DepthEnabled
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

DepthStencilState NoDepth
{
	DepthEnable = FALSE;
};

//SAMPLER STATES
//**************
SamplerState gTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
 	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
};

//LIGHT
//*****
float3 gLightDirection :DIRECTION;

//DIFFUSE
//*******
bool gUseTextureDiffuse;
float4 gColorDiffuse;
Texture2D gTextureDiffuse;

//SPECULAR
//********
float4 gColorSpecular;
Texture2D gTextureSpecularIntensity;
bool gUseTextureSpecularIntensity;
int gShininess;

//AMBIENT
//*******
float4 gColorAmbient;
float gAmbientIntensity;

//NORMAL MAPPING
//**************
bool gFlipGreenChannel;
bool gUseTextureNormal;
Texture2D gTextureNormal;
float gNormalMappingIntensity;

//ENVIRONMENT MAPPING
//*******************
TextureCube gCubeEnvironment;
bool gUseEnvironmentMapping;
float gReflectionStrength;
float gRefractionStrength;
float gRefractionIndex;

//FRESNEL FALLOFF
//***************
bool gUseFresnelFalloff;
float4 gColorFresnel;
float gFresnelPower;
float gFresnelMultiplier;
float gFresnelHardness;

//OPACITY
//***************
float gOpacityIntensity;
bool gUseTextureOpacity;
Texture2D gTextureOpacity;

//SPECULAR MODELS
//***************
bool gUseSpecularBlinn;
bool gUseSpecularPhong;

//VS IN & OUT
//***********
struct VS_Input
{
	float3 Position: POSITION;
	float3 Normal: NORMAL;
	float3 Tangent: TANGENT;
	float2 TexCoord: TEXCOORD0;
};

struct VS_Output
{
	float4 Position: SV_POSITION;
	float4 WorldPosition: COLOR0;
	float3 Normal: NORMAL;
	float3 Tangent: TANGENT;
	float2 TexCoord: TEXCOORD0;
};

float3 CalculateSpecularBlinn(float3 viewDirection, float3 normal, float2 texCoord)
{
	float3 halfVector = -normalize(viewDirection + gLightDirection);
	float3 hDn = saturate(dot(halfVector, normal));
	float3 specColor = gColorSpecular;
	
	if (gUseTextureSpecularIntensity)
		specColor = gTextureSpecularIntensity.Sample(gTextureSampler, texCoord).rgb;
	
	return specColor * pow(hDn, gShininess);
}

float3 CalculateSpecularPhong(float3 viewDirection, float3 normal, float2 texCoord)
{
	float3 r = reflect(gLightDirection, normal);
	float3 rDvv = saturate(dot(r, -viewDirection));
	float3 specColor = gColorSpecular;
	
	if (gUseTextureSpecularIntensity)
		specColor = gTextureSpecularIntensity.Sample(gTextureSampler, texCoord).rgb;
	
	return specColor * pow(rDvv, gShininess);
}

float3 CalculateSpecular(float3 viewDirection, float3 normal, float2 texCoord)
{
	float3 specColor = float3(0,0,0);
	
	if (gUseSpecularBlinn)
		specColor = CalculateSpecularBlinn(viewDirection, normal, texCoord);
	else if (gUseSpecularPhong)
		specColor = CalculateSpecularPhong(viewDirection, normal, texCoord);
				
	return specColor;
}

float3 CalculateNormal(float3 tangent, float3 normal, float2 texCoord)
{
	float3 newNormal = normal;
	
	if (gUseTextureNormal)
	{
		float3 binormal = cross(normal, tangent);	
		if (gFlipGreenChannel)
			binormal = -binormal;
		float3x3 localAxis = float3x3(tangent, binormal, normal);
		float3 sampledNormal = gTextureNormal.Sample(gTextureSampler, texCoord).rgb;
		sampledNormal = sampledNormal * 2 - 1;
		sampledNormal *= float3(gNormalMappingIntensity, gNormalMappingIntensity, 1.f);
		newNormal = normalize(mul(sampledNormal, localAxis));
	}

	return newNormal;
}

float3 CalculateDiffuse(float3 normal, float2 texCoord)
{
	float diffuseStrength = saturate(dot(-gLightDirection, normal));
	float3 diffuse = gColorDiffuse * diffuseStrength;
	
	if (gUseTextureDiffuse)
	{
		float3 diffuseSample = gTextureDiffuse.Sample(gTextureSampler, texCoord).rgb;
		diffuse *= diffuseSample;
	}
	
	return diffuse;
}

float3 CalculateFresnelFalloff(float3 normal, float3 viewDirection, float3 environmentColor)
{
	if (gUseFresnelFalloff)
	{
		float f = pow((1 - saturate(abs(dot(normal, viewDirection)))), gFresnelPower) * gFresnelMultiplier;
		float fMask = pow((1 - saturate(dot(float3(0.f,-1.f,0.f), normal))), gFresnelHardness);
		environmentColor *= f * fMask;
	}
	
	return environmentColor;
}

float3 CalculateEnvironment(float3 viewDirection, float3 normal)
{	
	if (gUseEnvironmentMapping)
	{
		float3 reflected = reflect(viewDirection, normal);
		float3 reflectedColor = gCubeEnvironment.Sample(gTextureSampler, reflected).rgb;
		float3 refracted = refract(viewDirection, normal, gRefractionIndex);
		float3 refractedColor = gCubeEnvironment.Sample(gTextureSampler, refracted).rgb;
		return (reflectedColor * gReflectionStrength) + (refractedColor * gRefractionStrength);
	}
	
	return gColorFresnel.rgb;
}

float CalculateOpacity(float2 texCoord)
{
	float opacity = gOpacityIntensity;
	
	if (gUseTextureOpacity)
		opacity *= gTextureOpacity.Sample(gTextureSampler, texCoord).r;
	
	return opacity;
}

// The main vertex shader
VS_Output MainVS(VS_Input input) {
	
	VS_Output output = (VS_Output)0;
	
	output.Position = mul(float4(input.Position, 1.0), gMatrixWVP);
	output.WorldPosition = mul(float4(input.Position,1.0), gMatrixWorld);
	output.Normal = mul(input.Normal, (float3x3)gMatrixWorld);
	output.Tangent = mul(input.Tangent, (float3x3)gMatrixWorld);
	output.TexCoord = input.TexCoord;
	
	return output;
}

// The main pixel shader
float4 MainPS(VS_Output input) : SV_TARGET {
	// NORMALIZE
	input.Normal = normalize(input.Normal);
	input.Tangent = normalize(input.Tangent);
	
	float3 viewDirection = normalize(input.WorldPosition.xyz - gMatrixViewInverse[3].xyz);
	
	//NORMAL
	float3 newNormal = CalculateNormal(input.Tangent, input.Normal, input.TexCoord);
		
	//SPECULAR
	float3 specColor = CalculateSpecular(viewDirection, newNormal, input.TexCoord);
		
	//DIFFUSE
	float3 diffColor = CalculateDiffuse(newNormal, input.TexCoord);
		
	//AMBIENT
	float3 ambientColor = gColorAmbient * gAmbientIntensity;
		
	//ENVIRONMENT MAPPING
	float3 environmentColor = CalculateEnvironment(viewDirection, newNormal);
	
	//FRESNEL FALLOFF
	environmentColor = CalculateFresnelFalloff(newNormal, viewDirection, environmentColor);
		
	//FINAL COLOR CALCULATION
	float3 finalColor = diffColor + specColor + environmentColor + ambientColor;
	
	//OPACITY
	float opacity = CalculateOpacity(input.TexCoord);
	
	return float4(finalColor,opacity);
}

// Default Technique
technique10 WithAlphaBlending {
	pass p0 {
		SetRasterizerState(gRS_FrontCulling);
		SetBlendState(EnableBlending,float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetDepthStencilState(NoDepth, 0);
		SetVertexShader(CompileShader(vs_4_0, MainVS()));
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, MainPS()));
	}
}

// Default Technique
technique10 WithoutAlphaBlending {
	pass p0 {
		SetRasterizerState(gRS_FrontCulling);
		SetBlendState(DisableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetDepthStencilState(DepthEnabled, 0);
		SetVertexShader(CompileShader(vs_4_0, MainVS()));
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, MainPS()));
	}
}

