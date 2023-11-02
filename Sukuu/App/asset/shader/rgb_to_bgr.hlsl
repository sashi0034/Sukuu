//
//	Textures
//
Texture2D g_texture0 : register(t0);
SamplerState g_sampler0 : register(s0);

namespace s3d
{
    //
    //	VS Output / PS Input
    //
    struct PSInput
    {
        float4 position : SV_POSITION;
        float4 color : COLOR0;
        float2 uv : TEXCOORD0;
    };
}

//
//	Constant Buffer
//
cbuffer PSConstants2D : register(b0)
{
    float4 g_colorAdd;
    float4 g_sdfParam;
    float4 g_sdfOutlineColor;
    float4 g_sdfShadowColor;
    float4 g_internal;
}

cbuffer RgbToBgrCb : register(b1)
{
    float g_rate;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    const float4 texColor = g_texture0.Sample(g_sampler0, input.uv);
    float4 newColor;

    newColor.r = texColor.r * (1 - g_rate) + texColor.b * g_rate;
    newColor.g = texColor.g;
    newColor.b = texColor.b * (1 - g_rate) + texColor.r * g_rate;
    newColor.a = texColor.a;

    return (newColor * input.color) + g_colorAdd;
}
