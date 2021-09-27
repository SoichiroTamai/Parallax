// カメラやライトのデータ
#include "../inc_Common.hlsli"
// 共通データ
#include "inc_ModelShader.hlsli"


//==================================
// 頂点シェーダー
//==================================
VSOutput main(
	float4 pos		: POSITION,		// 表示座標
	float2 uv		: TEXCOORD0,	// UV座標
	float3 normal	: NORMAL,		// 法線
	float4 color	: COLOR,		// 頂点色
	float3 tangent	: TANGENT		// 接線
)
{
	VSOutput Out;

	// 頂点座標を法線方向に、少しずらす
	pos.xyz = pos.xyz + normal * 0.03;

	// 3D頂点座標を2Dへ変換
	Out.Pos = mul(pos, g_mW);		// ワールド行列で変換してから、
	Out.Pos = mul(Out.Pos, g_mV);	// カメラの逆行列(ビュー行列)で変換して、
	Out.Pos = mul(Out.Pos, g_mP);	// 射影行列で変換

	return Out;
}