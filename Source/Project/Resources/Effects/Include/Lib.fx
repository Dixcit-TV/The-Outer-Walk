
//SHADOW MAPPING

float2 TexOffset(float u, float v, float width, float height)
{
	//TODO: return offseted value
	return float2( u * 1.0f/width, v * 1.0f/height );
}

float EvaluateShadowMap(Texture2D shadowMap, SamplerComparisonState scs, float4 lpos, float sampleCount = 4.f, float shadowBias = 0.01f)
{
	//TODO: complete
	lpos /= lpos.w;
	
	if( lpos.x < -1.0f || lpos.x > 1.0f ||
        lpos.y < -1.0f || lpos.y > 1.0f ||
        lpos.z < 0.0f  || lpos.z > 1.0f ) 
		return 0.f;
	
	lpos.x = lpos.x/2 + 0.5;
    lpos.y = lpos.y/-2 + 0.5;
	
	lpos.z -= shadowBias;
	
	float w;
	float h;
	shadowMap.GetDimensions(w, h);
	//PCF sampling for shadow map
    float sum = 0;
	float nBound = (sampleCount - 1) * 0.5f;
    float x, y;
 
    for (y = -nBound; y <= nBound; y += 1.0)
    {
        for (x = -nBound; x <= nBound; x += 1.0)
        {
            sum += shadowMap.SampleCmpLevelZero(scs, lpos.xy + TexOffset(x,y,w,h), lpos.z );
        }
    }
 
    return sum / (sampleCount * sampleCount);
}

float EvaluateShadowMapLargePCF(Texture2D shadowMap, SamplerComparisonState scs, float4 lpos, float sampleCount = 4.f, float shadowBias = 0.01f)
{
	//TODO: complete
	lpos /= lpos.w;

	if (lpos.x < -1.0f || lpos.x > 1.0f ||
		lpos.y < -1.0f || lpos.y > 1.0f ||
		lpos.z < 0.0f || lpos.z > 1.0f)
		return 0.f;

	lpos.x = lpos.x / 2 + 0.5;
	lpos.y = lpos.y / -2 + 0.5;

	lpos.z -= shadowBias;

	float w;
	float h;
	shadowMap.GetDimensions(w, h);

	float3 vShadowDDX = ddx(lpos.xyz);
	float3 vShadowDDY = ddy(lpos.xyz);
	float2x2 matScreentoShadow = float2x2(vShadowDDX.xy, vShadowDDY.xy);
	float invDet = matScreentoShadow[0].x * matScreentoShadow[1].y - matScreentoShadow[0].y * matScreentoShadow[1].x;
	float2x2 matShadowToScreen = float2x2 (
		matScreentoShadow._22 * invDet,
		matScreentoShadow._12 * -invDet,
		matScreentoShadow._21 * -invDet,
		matScreentoShadow._11 * invDet);

	float2 texSize = TexOffset(1.f, 1.f, w, h);

	float2 vRightShadowTexelLocation = float2(texSize.x, 0.0f);
	float2 vUpShadowTexelLocation = float2(0.0f, texSize.y);
	float2 vRightTexelDepthRatio = mul(vRightShadowTexelLocation,
		matShadowToScreen);
	float2 vUpTexelDepthRatio = mul(vUpShadowTexelLocation,
		matShadowToScreen);

	float fUpTexelDepthDelta =
		vUpTexelDepthRatio.x * vShadowDDX.z
		+ vUpTexelDepthRatio.y * vShadowDDY.z;
	float fRightTexelDepthDelta =
		vRightTexelDepthRatio.x * vShadowDDX.z
		+ vRightTexelDepthRatio.y * vShadowDDY.z;

	//PCF sampling for shadow map
	float sum = 0;
	float nBound = (sampleCount - 1) * 0.5f;
	float x, y;
	float depth = lpos.z;

	for (y = -nBound; y <= nBound; y += 1.0)
	{
		for (x = -nBound; x <= nBound; x += 1.0)
		{
			depth += fRightTexelDepthDelta * x + fRightTexelDepthDelta * y;
			sum += shadowMap.SampleCmpLevelZero(scs, lpos.xy + float2(texSize.x * x, texSize.y * y), depth);
		}
	}

	return sum / (sampleCount * sampleCount);
}

float remap01(float x, float min, float max)
{
	return saturate((x - min) / (max - min));
}

float smoothMax(float a, float b, float k) {
	k = min(0, -k);
	float h = max(0, min(1, (b - a + k) / (2 * k)));
	return a * h + b * (1 - h) - k * h * (1 - h);
}

float smoothMin(float a, float b, float k) {
	k = max(0, k);
	// https://www.iquilezles.org/www/articles/smin/smin.htm
	float h = max(0, min(1, (b - a + k) / (2 * k)));
	return a * h + b * (1 - h) - k * h * (1 - h);
}
