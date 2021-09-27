#pragma once

//============================================================
//
// 基本シェーダ
//
//============================================================
class StandardShader
{
	struct cbObject;
	struct cbMaterial;
public:


	//================================================
	// 設定・取得
	//================================================

	// ワールド行列セット
	void SetWorldMatrix(const Math::Matrix& m)
	{
		m_cb0.Work().mW = m;
	}

	// UVタイリング設定
	void SetUVTiling(const Math::Vector2& tiling)
	{
		m_cb0.Work().UVTiling = tiling;
	}
	// UVオフセット設定
	void SetUVOffset(const Math::Vector2& offset)
	{
		m_cb0.Work().UVOffset = offset;
	}

	// ライト有効/無効
	void SetLightEnable(bool enable)
	{
		m_cb0.Work().LightEnable = enable;
	}

	// フォグ有効/無効
	void SetFogEnable(bool enable)
	{
		m_cb0.Work().FogEnable = enable;
	}

	// アルファテストの閾値設定
	// ・alpha	… この値以下のアルファ値のピクセルは描画されなくなる
	void SetAlphaTestThreshold(float alpha)
	{
		m_cb0.Work().AlphaTestThreshold = alpha;
	}

	// Set系でセットしたデータを、実際に定数バッファへ書き込む
	void WriteToCB()
	{
		m_cb0.Write();
		m_cb1_Material.Write();
	}

	// Object用定数バッファをそのまま返す
	ConstantBuffer<cbObject>& ObjectCB()
	{
		return m_cb0;
	}

	// material用定数バッファをそのまま返す
	ConstantBuffer<cbMaterial>& MaterialCB()
	{
		return m_cb1_Material;
	}

	//================================================
	// 描画
	//================================================

	// このシェーダをデバイスへセット
	void SetToDevice();

	// メッシュ描画
	// ・mesh			… 描画するメッシュ
	// ・materials		… 使用する材質配列
	void DrawMesh(const Mesh* mesh, const std::vector<Material>& materials);

	//================================================
	// 初期化・解放
	//================================================

	// 初期化
	bool Init();
	// 解放
	void Release();
	// 
	~StandardShader()
	{
		Release();
	}


private:

	// 3Dモデル用シェーダ
	ID3D11VertexShader*	m_VS = nullptr;				// 頂点シェーダー
	ID3D11InputLayout*	m_inputLayout = nullptr;	// 頂点入力レイアウト

	ID3D11PixelShader*	m_PS = nullptr;				// ピクセルシェーダー

	// 定数バッファ
	//  ※定数バッファは、パッキング規則というものを厳守しなければならない
	//  <パッキング規則> 参考：https://docs.microsoft.com/ja-jp/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules
	//  ・構造体のサイズは16バイトの倍数にする。
	//  ・各項目(変数)は、16バイト境界をまたぐような場合、次のベクトルに配置される。

	// 定数バッファ(オブジェクト単位更新)
	struct cbObject
	{
		Math::Matrix		mW;		// ワールド行列　行列は16バイトx4バイトの64バイトなのでピッタリ。

		// UV操作
		Math::Vector2		UVOffset = { 0,0 };
		Math::Vector2		UVTiling = { 1,1 };

		// ライト有効
		int					LightEnable = 1;
		// フォグ有効
		int					FogEnable = 1;
		// アルファテストの閾値
		float				AlphaTestThreshold = 0;

		float tmp[1];
	};
	ConstantBuffer<cbObject>	m_cb0;

	// 定数バッファ(マテリアル単位更新)
	struct cbMaterial {
		Math::Vector4	BaseColor;
		Math::Vector3	Emissive;
		float			Metallic;
		float			Roughness;

		float			tmp[3];
	};
	ConstantBuffer<cbMaterial>	m_cb1_Material;

};


