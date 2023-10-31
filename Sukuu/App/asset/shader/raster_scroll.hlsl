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

cbuffer RasterScrollCb : register(b1)
{
    float g_phase;
    float g_amplitude;
    float g_freq;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    const float offset = g_amplitude * sin(g_phase + g_freq * input.uv.y);

    float x = input.uv.x + offset;
    if (x < 0) x += 1;
    else if (x > 1) x -= 1;

    float4 texColor = g_texture0.Sample(g_sampler0, float2(x, input.uv.y));

    return (texColor * input.color) + g_colorAdd;
}
