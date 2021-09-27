#include "../inc_Common.hlsli"
#include "inc_EffectShader.hlsli"

// テクスチャ
Texture2D g_tex : register(t0);

// サンプラ
SamplerState g_ss : register(s0);

//================================
// エフェクト用ピクセルシェーダ
//================================
float4 main(VSOutput In) : SV_Target0
{
    float4 color = g_tex.Sample(g_ss, In.UV) * In.Color;
    
    // アルファテスト
    if (color.a <= g_AlphaTestThreshold) discard;

    return color;
}
