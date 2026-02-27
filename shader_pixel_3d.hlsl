/*==============================================================================

   3D描画用ピクセルシェーダー [shader_pixel_3d.hlsl]
														 Author : Youhei Sato
														 Date   : 2025/09/10
--------------------------------------------------------------------------------

==============================================================================*/
// 定数バッファ
cbuffer PS_CONSTANT_BUFFER : register(b0)
{
    float4 diffuse_color;
};

cbuffer PS_CONSTANT_BUFFER : register(b1)
{
    float4 ambient_color;
};

cbuffer PS_CONSTANT_BUFFER : register(b2)
{
    float4 directional_world_vector;
    float4 directional_color;
}

cbuffer PS_CONSTANT_BUFFER : register(b3)
{
    float3 eye_posW;
    float specular_power;
    float4 specular_color;
};

struct PointLight
{
    float3 posW;
    float range;
    float4 color;
};

cbuffer PS_CONSTANT_BUFFER : register(b4)
{
    PointLight point_light[4];
    int point_light_count;
    float3 point_light_dummy;
};

cbuffer PS_CONSTANT_BUFFER : register(b5)
{
    float3 lim_light_color;
    float lim_light_power;
};

struct PS_IN
{
    float4 posH        : SV_POSITION;
    float4 posW        : POSITION0;
    float4 posLightWVP : POSITION1;
    float4 normalW     : NORMAL0;
    float4 color       : COLOR0;
    float2 uv          : TEXCOORD0;
};

Texture2D tex0 : register(t0); // テクスチャ
Texture2D tex2 : register(t2); // 深度情報テクスチャ
SamplerState samp : register(s0); // テクスチャサンプラ
//SamplerState samp : register(s0); // テクスチャサンプラ

float4 main(PS_IN pi) : SV_TARGET
{
    // 材質の色
   float3 material_color = tex0.Sample(samp, pi.uv).rgb * pi.color.rgb * diffuse_color.rgb;
   // float3 material_color = tex0.Sample(samp, float2(0, 0)).rgb;
  
    
    // 並行光源（ディフューズライト）
    float4 normalW = normalize(pi.normalW);
    // float dl = max(0.0f, dot(-directional_world_vector, normalW));
    float dl = (dot(-directional_world_vector, normalW) + 1.0f) * 0.5f;
    float3 diffuse = material_color * directional_color.rgb * dl;
    
    // 環境光（アンビエントライト）
    float3 ambient = material_color * ambient_color.rgb;
    // float3 ambient = { 0.0f, 0.0f, 0.0f };
    
    // スペキュラ
    float3 toEye = normalize(eye_posW - pi.posW.xyz);
    float3 r = reflect(directional_world_vector, normalW).xyz;
    float t = pow(max(dot(r, toEye), 0.0f), specular_power);
    float3 specular = specular_color.rgb * t;
    
    float alpha = tex0.Sample(samp, pi.uv).a * pi.color.a * diffuse_color.a;
    float3 color = ambient + diffuse + specular; // 最終的な我々の目に届く色
    
    // リムライト実験サンプルコード
    float lim = 1.0f - max(dot(normalW.xyz, toEye), 0.0f);
    lim = pow(lim, lim_light_power);
    color += lim_light_color * lim;
     
    // 点光源（ポイントライト）のサンプルコード
    
    for (int i = 0; i < point_light_count; i++)
    {
        // 点光源から面（ピクセル）へのベクトルを算出
        float3 lightToPixel = pi.posW.xyz - point_light[i].posW;
        
        // 面（ピクセル）とライトとの距離を測る
        float D = length(lightToPixel);
    
        // 影響力の計算
        float A = pow(max(1.0f - 1.0f / point_light[i].range * D, 0.0f), 2.0f);
        // range = 400 length = 0   ... A = 1       A * A = 1
        //                    = 100 ... A = 0.75          = 0.5625
        //                    = 200 ... A = 0.5           = 0.25
        //                    = 300 ... A = 0.25          = 0.0625
        //                    = 400 ... A = 0             = 0
    
        // 点光源と面（ピクセル）との向きを考慮に入れる
        
        float dl = max(0.0f, dot(-normalize(lightToPixel), normalW.xyz));
        // float dl = (dot(-directional_world_vector, normalW) + 1.0f) * 0.5f;
       
        // 点光源の影響を加算する
        color += material_color * point_light[i].color.rgb * A * dl;
        
        // 点光源のスペキュラ
        float3 r = reflect(normalize(lightToPixel), normalW.xyz);
        float t = pow(max(dot(r, toEye), 0.0f), specular_power);
        
        // 点光源のスペキュラを加算する
        color += point_light[i].color.rgb * t;
    }
    
    //影の計算
    float2 shadowmap_uv = pi.posLightWVP.xy / pi.posLightWVP.w;
    shadowmap_uv = shadowmap_uv * float2(0.5f, -0.5f) + 0.5f;
    
    float depthmap_z = tex2.Sample(samp, shadowmap_uv).r;
    
    float shadowmap_z = pi.posLightWVP.z / pi.posLightWVP.w;
    
    if (shadowmap_z > depthmap_z)
    {
        color = color * 0.5f;
    }
    
    return float4(color, alpha);
}
