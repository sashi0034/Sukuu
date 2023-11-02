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

cbuffer GrayscaleCb : register(b1)
{
    float g_rate;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    float4 texColor = g_texture0.Sample(g_sampler0, input.uv);

    const float gray = dot(texColor.rgb, float3(0.299, 0.587, 0.114));

    texColor.rgb = float3(gray, gray, gray) * g_rate + texColor.rgb * (1 - g_rate);

    return (texColor * input.color) + g_colorAdd;
}
