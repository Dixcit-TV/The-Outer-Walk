//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror;
    AddressV = Mirror;
};

Texture2D gTexture;

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
	// Set the TexCoord
	output.TexCoord = input.TexCoord;
	
	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input): SV_Target
{
	// Step 1: find the dimensions of the texture (the texture has a method for that)	
	float w;
	float h;
	gTexture.GetDimensions(w, h);
	// Step 2: calculate dx and dy (UV space for 1 pixel)	
	float dx = 1 / w;
	float dy = 1 / h;
	// Step 3: Create a double for loop (5 iterations each)
	float nSize = 5;
    float3 color = float3(0.f, 0.f, 0.f);
	float nBound = (nSize - 1) * 0.5f;
    float x, y;
	float2 offset;
 
    for (y = -nBound; y <= nBound; y += 1.0)
    {
        for (x = -nBound; x <= nBound; x += 1.0)
        {
			offset.x = 2 * dx * x;
			offset.y = 2 * dy * y;
            color += gTexture.Sample(samPoint, input.TexCoord + offset).rgb;
        }
    }

	color /= (nSize * nSize);
	return float4(color, 1.0f);
}


//TECHNIQUE
//---------
technique11 Blur
{
    pass P0
    {
		// Set states...
		SetRasterizerState(BackCulling);
		SetDepthStencilState(EnableDepth, 0);
		
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}