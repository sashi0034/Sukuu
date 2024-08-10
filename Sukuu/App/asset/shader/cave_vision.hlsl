#define PI 3.14159265358979323846
//
//	Textures
//
Texture2D g_maskTexture : register(t0);
Texture2D g_texture1 : register(t1);
Texture2D g_texture2 : register(t2);
SamplerState g_maskSampler : register(s0);
SamplerState g_sampler1 : register(s1);
SamplerState g_sampler2 : register(s2);

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

cbuffer CaveVisionCb : register(b1)
{
    float g_animRate;
    bool g_masked;
    float2 g_positionOffset;
}

float2 calcUvTex1(s3d::PSInput input)
{
    const float2 vecOne = {1.0f, 1.0f};
    const float uvStep = 96.0;
    const float2 xy = g_positionOffset + input.position.xy;

    const float scrollSpeed = 2.0;
    float2 uv = ((xy % uvStep) / uvStep) + ((scrollSpeed * g_animRate) % 1.0) * vecOne;

    const float waveStep = 2.0;
    const float waveY = sin(xy.y / 180.0 * PI * waveStep);
    const float waveSpeed = 1.5;
    const float waveMax = 0.5;
    const float waveAmount = waveMax * sin(g_animRate * PI * waveSpeed);
    uv.x = uv.x + waveAmount * waveY;

    uv = (uv + vecOne) % 1.0;
    return uv;
}

float2 calcUvTex2(s3d::PSInput input)
{
    const float2 vecOne = {1.0f, 1.0f};
    const float uvStep = 96.0 * 2;
    const float2 xy = g_positionOffset + input.position.xy;

    float2 uv = ((xy % uvStep) / uvStep);

    const float oscillationSpeed = 2.7;
    const float oscillationAmount = 0.3;
    uv.x = uv.x + oscillationAmount * sin(
        ((xy.x + xy.y) / uvStep + g_animRate * oscillationSpeed) * PI);

    uv = (uv + vecOne) % 1.0;
    return uv;
}

float square(float x)
{
    return x * x;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    float4 color0 = input.color;

    const float2 uv1 = calcUvTex1(input);
    const float2 uv2 = calcUvTex2(input);

    float4 color1 = g_texture1.Sample(g_sampler1, uv1);
    float4 color2 = g_texture2.Sample(g_sampler2, uv2);

    color0.rgb = (color0.rgb * 0.2 + color1.rgb * 0.4 + color2.rgb * 0.4);
    if (g_masked) color0.a = 1 - g_maskTexture.Sample(g_maskSampler, input.uv).g;

    return (color0 * input.color) + g_colorAdd;
}
