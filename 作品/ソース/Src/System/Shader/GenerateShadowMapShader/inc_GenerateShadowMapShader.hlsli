// 定数バッファ
cbuffer cbObject : register(b0)
{
    row_major float4x4 g_mW;        // ワールド行列
    row_major float4x4 g_mLightVP;  // ライトカメラのビュー行列*射影行列
};

// VS出力用構造体
struct VSOutput
{
    float4 Pos      : SV_Position;  // 射影座標
    float2 UV       : TEXCOORD0;    // UV座標
    float4 wvpPos   : TEXCOORD1;    // 射影座標(PosはPSに来た時にはScreen座標になってしまうため別途渡す必要がある)
};