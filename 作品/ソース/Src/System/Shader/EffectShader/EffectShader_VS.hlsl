#include "../inc_Common.hlsli"
#include "inc_EffectShader.hlsli"

//================================
// エフェクト用 頂点シェーダ
//================================
VSOutput main(float4 pos : POSITION, // 頂点座標
              float2 uv : TEXCOORD0,  // テクスチャUV座標
              float4 color : COLOR    // 色
){
    VSOutput Out;

    // 座標変換
    Out.Pos = mul(pos, g_mW);       // ローカル座標系 -> ワールド座標系へ変換
    Out.Pos = mul(Out.Pos, g_mV);   // ワールド座標系 -> ビュー座標系へ変換
    Out.Pos = mul(Out.Pos, g_mP);   // ビュー座標系 -> 射影座標系へ変換

    // UV座標
    Out.UV = uv * g_UVTiling + g_UVOffset;

    // 色
    Out.Color = color;

    // 出力
    return Out;
}
