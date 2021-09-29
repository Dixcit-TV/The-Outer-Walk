float4x4 gWorld;
float4x4 gViewInverse;
float4x4 gProjInverse;
float gPlanetRadius;

//To be replaced by parameters
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
float gLightIntensity = 5.f;
float gSpecularReflectance = 1.f;
float gShininess = 100.f;
float gTime;
float gNormalMapScaling = 0.05f;

cbuffer OceanSettings
{
	float3 gShallowColor;
	float3 gDeepColor;
	float gMaxDepth;
};

Texture2D gDepthTexture;
Texture2D gColorTexture;
Texture2D gNormalMap;

float MAXFLOAT = 3.4028235f * pow(10, 38);
float PI = 3.14159265359f;

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerComparisonState cmpSampler
{
	// sampler state
	Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
	AddressU = MIRROR;
	AddressV = MIRROR;

	// sampler comparison state
	ComparisonFunc = LESS_EQUAL;
};

/// Create Depth Stencil State (ENABLE DEPTH WRITING)
/// Create Rasterizer State (Backface culling) 
RasterizerState BackCulling
{
	CullMode = BACK;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD0;

};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD1;
};


//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	// Set the Position
	output.Position = float4(input.Position, 1.f);
	output.TexCoord = input.TexCoord;

	return output;
}


float2 SphereIntersection(float3 rayStart, float3 rayDir, float3 center, float radius)
{
	float3 rayToCenter = rayStart - center;
	float b = 2 * dot(rayDir, rayToCenter);
	float c = dot(rayToCenter, rayToCenter) - radius * radius;

	float d = b * b - 4 * c;
	if (d >= 0.f)
	{
		float sqrtD = sqrt(d);
		float t1 = max(0.f, (-b - sqrtD) / 2.f);
		float t2 = (-b + sqrtD) / 2.f;

		return float2(t1, t2 - t1);
	}

	return float2(MAXFLOAT, 0.f);
}

float remap01(float x, float min, float max)
{
	return saturate((x - min) / (max - min));
}

float ShadowMaskIndex(float3 normal, float3 viewVector, float roughness)
{
	float d = dot(normal, viewVector);
	float k = (roughness + 1) * (roughness + 1) / 8.f;
	return d / (d * (1 - k) + k);
}

float3 TriplanarNormalMapping_UDN(Texture2D map, SamplerState samp, float3 normal, float3 worldPos, float2 offset, float mapScaling)
{
	//Tri-Planar Normal mapping (waves)
	float3 weights = abs(normal);
	weights = max(weights - 0.2f, 0.f);
	weights /= dot(weights, float3(1.f, 1.f, 1.f));

	float3 normalX = map.Sample(samp, worldPos.yz * mapScaling + offset).rgb;
	float3 normalY = map.Sample(samp, worldPos.xz * mapScaling + offset).rgb;
	float3 normalZ = map.Sample(samp, worldPos.xy * mapScaling + offset).rgb;

	//UDN blending
	normalX = float3(normalX.xy + normal.zy, normal.x);
	normalY = float3(normalY.xy + normal.xz, normal.y);
	normalZ = float3(normalZ.xy + normal.xy, normal.z);

	normal = normalX.zyx * weights.x + normalY.xzy * weights.y + normalZ.xyz * weights.z;
	return normalize(normal);
}

//PIXEL SHADER
//------------
//Depth to viewSpace https://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/
//Ray cast the ocean Surface (15min 30) https://www.youtube.com/watch?v=lctXaT9pxA0
//Triplanar mapping & normal map https://bgolus.medium.com/normal-mapping-for-a-triplanar-shader-10bf39dca05a
float4 PS(PS_INPUT input) : SV_Target
{
	float depth = gDepthTexture.Sample(samPoint, input.TexCoord).r;
	//Set UV coordinates to NDC
	float x = input.TexCoord.x * 2 - 1;
	float y = (1 - input.TexCoord.y) * 2 - 1;
	//calculate view space from NDC XY and Depth value
	float4 pos = float4(x, y, depth, 1.f);
	pos = mul(pos, gProjInverse);
	pos /= pos.w;

	//Transform to world space
	float3 viewVector = mul(pos.xyz, (float3x3)gViewInverse);
	float vvLength = length(viewVector);
	float3 rayDir = viewVector / vvLength;
	float3 oceanCenter = gWorld[3].xyz;
	float3 cameraPos = gViewInverse[3].xyz;

	//Ray trace ocean sphere surface: water level being always 0 (planet radius)
	float2 ocenHitInfo = SphereIntersection(cameraPos, rayDir, oceanCenter, gPlanetRadius);
	//calculate ocean depth we are looking through
	float oceanDepth = min(ocenHitInfo.y, vvLength - ocenHitInfo.x);

	float3 color = gColorTexture.Sample(samPoint, input.TexCoord).rgb;

	if (oceanDepth > 0.f)
	{
		oceanDepth = saturate(remap01(oceanDepth, 0.f, gMaxDepth) + 0.33f); // Reach deep color sooner

		float3 oceanColor = lerp(gShallowColor, gDeepColor, oceanDepth);

		float3 viewVectOut = -rayDir;
		float3 pixelWorldPos = cameraPos + rayDir * ocenHitInfo.x - oceanCenter;
		float3 pixelNormal = normalize(pixelWorldPos);
	
		float3 n1 = TriplanarNormalMapping_UDN(gNormalMap, samPoint, pixelNormal, pixelWorldPos, float2(gTime, gTime), gNormalMapScaling);
		float3 n2 = TriplanarNormalMapping_UDN(gNormalMap, samPoint, pixelNormal, pixelWorldPos, float2(gTime, -gTime), gNormalMapScaling * 0.5f);
		pixelNormal = normalize(lerp(pixelNormal, normalize(n1 + n2), 0.75f)); //Use only 75% of the wave normal map

		//Diffuse Color
		float diffuseStrength = dot(pixelNormal, -gLightDirection);
		oceanColor *= saturate(diffuseStrength) * gLightIntensity / PI;


		oceanColor *= oceanDepth;
		oceanColor += color * (1 - oceanDepth);
		//oceanColor *= saturate(length(color));

		//Phong Specular
		float3 r = reflect(gLightDirection, pixelNormal);
		float specularStrength = saturate(dot(viewVectOut, r));
		float3 specular = float3(gSpecularReflectance, gSpecularReflectance, gSpecularReflectance) * gLightIntensity;
		specular *= pow(specularStrength, gShininess);

		return float4(oceanColor + specular, 1.f);
	}

	return float4(color, 1.f);
}


//TECHNIQUE
//---------
technique11 Ocean
{
	pass P0
	{
		// Set states...
		SetRasterizerState(BackCulling);
		SetDepthStencilState(EnableDepth, 0);

		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}