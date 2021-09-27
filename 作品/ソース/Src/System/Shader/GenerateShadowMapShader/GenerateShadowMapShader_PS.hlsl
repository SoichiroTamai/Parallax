#include "inc_GenerateShadowMapShader.hlsli"

/*
    アルファ値が一定以下ならそのピクセルは描画しないというdiscard命令を実行
*/

Texture2D       g_baseTex   : register(t0);
SamplerState    g_ss        : register(s0);

// ピクセルシェーダ
float4 main(VSOutput In) : SV_Target0
{
    // アルファ値が一定以下のものは捨てる
    float4 texCol = g_baseTex.Sample(g_ss, In.UV);
    // アルファテスト
    if (texCol.a < 0.1)
    {
        discard;
    }

    // R成分に射影座標系での深度値(最近接距離：0.0 ～ 最遠方距離：1.0)を出力
    // ※射影座標系のxyzはwで除算する必要がある
    return float4(In.wvpPos.z / In.wvpPos.w, 0, 0, 1);
}