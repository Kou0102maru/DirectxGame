/*==============================================================================

   メッシュフィールド描画用頂点シェーダー [shader_vertex_field.hlsl]
														 Author : Youhei Sato
														 Date   : 2025/10/20
--------------------------------------------------------------------------------

==============================================================================*/

// 定数バッファ
cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    float4x4 world;
};

cbuffer VS_CONSTANT_BUFFER : register(b1)
{
    float4x4 view;
};

cbuffer VS_CONSTANT_BUFFER : register(b2)
{
    float4x4 proj;
};

cbuffer VS_CONSTANT_BUFFER : register(b3)
{
    float4x4 ligth_view_proj;
};


struct VS_IN
{
    float4 posL : POSITION0;
    float4 normalL : NORMAL0;
    float4 blend : COLOR0;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float4 posW : POSITION0;
    float4 posLightWVP : POSITION1;
    float4 normalW : NORMAL0;
    float4 blend : COLOR0;
    float2 uv : TEXCOORD0;
};

//=============================================================================
// 頂点シェーダ
//=============================================================================
VS_OUT main(VS_IN vi)
{
    VS_OUT vo;
    
    // 座標変換
    float4x4 mtxWV = mul(world, view);
    float4x4 mtxWVP = mul(mtxWV, proj);
    vo.posH = mul(vi.posL, mtxWVP);
    
   //ライトビュープロジェクション空間へ座標返還
    vo.posLightWVP = mul(vi.posL, mul(world, ligth_view_proj));
    
    // 普通のワールド変換行列×
    // ワールド変換行列の転置逆行列○
    float4 normalW = mul(float4(vi.normalL.xyz, 0.0f), world);
    vo.normalW = normalW;
    vo.posW = mul(vi.posL, world);
    
    vo.blend = vi.blend; // 地面のテクスチャのブレンド値はそのままパススルー
    vo.uv = vi.uv;
    
    return vo;
}
