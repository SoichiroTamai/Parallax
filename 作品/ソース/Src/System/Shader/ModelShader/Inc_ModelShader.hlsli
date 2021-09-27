//===========================================
//
// 定数バッファ
//  ゲームプログラム側から持ってくるデータ
//
//===========================================

// 定数バッファ(オブジェクト単位更新)
cbuffer cbObject : register(b0)
{
    // オブジェクト情報
    row_major float4x4 g_mW; // ワールド変換行列(DirectXは行優先、OpenGlなどは列優先)
};

// 頂点シェーダから出力するデータ
struct VSOutput
{
    float4 Pos      : SV_Position;  // 射影座標
    float2 UV       : TEXCOORD0;    // UV座標
    float4 Color    : TEXCOORD1;    // 色
    float3 wN       : TEXCOORD2;    // ワールド法線 (平行光等に使用)
    float3 wPos     : TEXCOORD3;    // ワールド3D座標
    float3 wT       : TEXCOORD4;    // ワールド接線(Tangent)
    float3 wB       : TEXCOORD5;    // ワールド従法線(Binormal)

};

//------------------------------
// 定数バッファ(マテリアル)
//------------------------------

// マテリアル構造体
struct Material
{
    float4  BaseColor;  // ベース色
    float   Metallic;   // 金属度
    float   Roughness;  // 粗さ
};

cbuffer cbMaterial : register(b1)
{
    Material g_Material;
};