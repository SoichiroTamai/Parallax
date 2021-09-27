//=====================================================
//
// 全シェーダ共通
//
//=====================================================

//------------------------------
// 定数バッファ(カメラ)
//------------------------------
cbuffer cbCamera : register(b7)
{
    // カメラ情報
    row_major float4x4  g_mV; // ビュー変換行列
    row_major float4x4  g_mP; // 射影変換行列
    float3              g_CamPos; // カメラ座標

	// フォグ
    int     g_DistanceFogEnable; // フォグ有効/無効フラグ
    float3  g_DistanceFogColor; // フォグ色
    float   g_DistanceFogDensity; // フォグ減衰率
};

//------------------------------
// 定数バッファ(ライト)
//------------------------------
cbuffer cbLight : register(b8)
{
    // 環境光
    float3  g_AmbientLight;

    // 平行光
    float3  g_DL_Dir;    // 光の方向
    float3  g_DL_Color;  // 光の色
    row_major float4x4 g_DL_mLightVP; // ライトカメラのビュー行列*射影行列

	//--------------
	// 点光
	//--------------
	// 使用数
	int4 g_PL_Cnt; // パッキング規則が大変なので、int4のxのみを使用する
	// データ
	struct PointLight
	{
		float3	Color;	// 色
		float	Radius;	// 半径
		float3	Pos;	// 座標
		float	tmp;
	};
	PointLight g_PL[100];
};
