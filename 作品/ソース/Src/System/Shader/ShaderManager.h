#pragma once

#include "EffectShader/EffectShader.h"
#include "StandardShader/StandardShader.h"
#include "SpriteShader/SpriteShader.h"
#include "ModelShader/ModelShader.h"
#include "GenerateShadowMapShader/GenerateShadowMapShader.h"

//==========================================================
//
// シェーダ関係をまとめたクラス
//
//==========================================================
class ShaderManager
{
public:

	//==============================================================
	//
	// 初期化・解放
	//
	//==============================================================

	// 初期化
	void Init();

	// 解放
	void Release();

	//==========================
	//
	// シェーダ
	//
	//==========================
	StandardShader			m_standardShader;		// 3Dモデル描画シェーダ
	EffectShader			m_effectShader;			// エフェクト描画シェーダ
	SpriteShader			m_spriteShader;			// 2Dテクスチャ描画シェーダ
	ModelShader				m_modelShader;			// モデルシェーダー
	GenerateShadowMapShader	m_genShadowMapShader;	// シャドウマップシェーダー

	//==========================
	//
	// カメラ関係
	//
	//==========================

	// カメラ定数バッファ用構造体
	struct cbCamera
	{
		Math::Matrix	mV;			// ビュー行列
		Math::Matrix	mP;			// 射影行列
		Math::Vector3	CamPos;		// カメラワールド座標

		int				DistanceFogEnable = 0;			// 距離フォグ有効
		Math::Vector3	DistanceFogColor = { 1, 1, 1 };	// 距離フォグ色
		float			DistanceFogDensity = 0.001f;	// 距離フォグ減衰率
	};

	// カメラ定数バッファ
	ConstantBuffer<cbCamera>	m_cb7_Camera;

	//==========================
	//
	// ライト関係
	//
	//==========================

	// ライト定数バッファ用構造体
	struct cbLight
	{
		//-----------------
		// 環境光
		//-----------------
		Math::Vector3	AmbientLight = {0.3f, 0.3f, 0.3f};
		float			tmp{};

		//-----------------
		// 平行光
		//-----------------
		Math::Vector3	DL_Dir = {0,-1,0};	// 光の方向
		float			tmp2{};
		
		Math::Vector3	DL_Color = {1,1,1};	// 光の色
		float			tmp3{};

		Matrix			DL_mLightVP;		// ライトカメラのビュー行列*射影行列

		//-----------------
		// 点光 (ポイントライト)
		//-----------------
		// 使用数
		int		PL_Cnt = 0;
		float	tmp4[3];

		// データ
		struct PointLight
		{
			Vec3	Color;	// 色
			float	Radius;	// 半径
			Vec3	Pos;	// 座標
			float	tmp;
		};
		std::array<PointLight, 100>	PL;
	};

	// 点光を追加　( 表示座標, 範囲, 色, 輝度 )
	void AddPointLight(const Vec3& pos, float radius, const Vec3& color, float brightess = 1.0f )
	{
		int idx = m_cb8_Light.GetWork().PL_Cnt;
		if (idx < (int)m_cb8_Light.GetWork().PL.size())
		{
			m_cb8_Light.Work().PL[idx].Pos = pos;
			m_cb8_Light.Work().PL[idx].Radius = radius;
			m_cb8_Light.Work().PL[idx].Color = color * brightess;

			m_cb8_Light.Work().PL_Cnt++;
		}
	}

	// 点光をリセット
	void ResetPointLight()
	{
		m_cb8_Light.Work().PL_Cnt = 0;
	}

	// ライト定数バッファ
	ConstantBuffer<cbLight>		m_cb8_Light;

	//==========================
	//
	// パイプラインステート
	//
	//==========================

	// 深度ステンシル
	ID3D11DepthStencilState* m_ds_ZEisable_ZWriteEnable  = nullptr;		// 奥行チェックあり, 奥行情報を残す
	ID3D11DepthStencilState* m_ds_ZDisable_ZWriteDisable = nullptr;		// 奥行チェックなし, 奥行情報を残さない
	ID3D11DepthStencilState* m_ds_ZEnable_ZWriteDisable	 = nullptr;		// 奥行チェックあり	, 奥行情報を残さない

	// ラスタライズステート
	ID3D11RasterizerState* m_rs_CullBack	= nullptr;	// 画面をカリングする	(描画する必要の無い裏面等は表示しない)
	ID3D11RasterizerState* m_rs_CullFront	= nullptr;	// アウトライン (裏面のみ描画)
	ID3D11RasterizerState* m_rs_CullNone	= nullptr;	// 画面をカリングしない (全方向描画する)

	// ブレンドステート
	ID3D11BlendState* m_bs_Alpha	= nullptr;
	ID3D11BlendState* m_bs_Add		= nullptr;

	//==========================
	//
	// その他
	//
	//==========================





//-------------------------------
// シングルトン
//-------------------------------
private:

	ShaderManager() {}

public:
	static ShaderManager &GetInstance() {
		static ShaderManager instance;
		return instance;
	}
};

#define SHADER ShaderManager::GetInstance()
