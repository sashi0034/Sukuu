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

cbuffer RgbShiftCb : register(b1)
{
    float2 g_center;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    const float2 screenCenter = g_center; // float2(0.5, 0.5);
    const float2 direction = normalize(input.uv - screenCenter);

    const float distance = length(input.uv - screenCenter);
    const float blurFactor = 0.01;
    const float blurAmount = distance * blurFactor;

    float4 color = float4(0, 0, 0, 0);

    const int n = 1;
    for (int i = -n; i <= n; ++i)
    {
        color += g_texture0.Sample(g_sampler0, input.uv + direction * blurAmount * i);
    }
    color /= (2 * n + 1);

    return color;
}
