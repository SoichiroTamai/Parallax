#include "../inc_Common.hlsli"
#include "inc_GenerateShadowMapShader.hlsli"

/*
	・頂点を座標変換する

	・PosはPSへ行くとScreen座標系になってしまい値が変わる為
	　VSでの射影座標をそのまま保存しておくためにwvpPosにも代入

	 ・座標が変えられるか変えられないかは、セマンティクスによる
*/

// 頂点シェーダ
VSOutput main(
	float4 pos : POSITION, // 書式：セマンティクス
	float2 uv : TEXCOORD0)
{
	VSOutput Out;

	// ライトカメラで射影変換
	Out.Pos = mul(pos, g_mW);
	Out.Pos = mul(Out.Pos, g_mLightVP);

	// 射影座標
	Out.wvpPos = Out.Pos;
	// UV座標
	Out.UV = uv;

	return Out;
}