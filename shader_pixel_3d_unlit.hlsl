/*==============================================================================

   ３D描画用頂点シェーダー（ライトなし） [shader_pixel_3d_unlit.hlsl]
														 Author : Youhei Sato
														 Date   : 2025/11/21
--------------------------------------------------------------------------------

==============================================================================*/
// 定数バッファ
cbuffer PS_CONSTANT_BUFFER : register(b0)
{
    float4 diffuse_color;
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D tex; // テクスチャ
SamplerState samp; // テクスチャサンプラ

float4 main(PS_IN pi) : SV_TARGET
{
    return tex.Sample(samp, pi.uv) * diffuse_color;
}
