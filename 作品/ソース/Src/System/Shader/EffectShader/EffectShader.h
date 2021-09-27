#pragma once


//============================================================
//
// エフェクトシェーダ
//
//============================================================
class EffectShader
{
public:

	// １頂点の形式
	struct Vertex
	{
		Math::Vector3 Pos;
		Math::Vector2 UV;
		Math::Vector4 Color;
	};

	//================================================
	// 取得・設定
	//================================================

	// テクスチャセット
	void SetTexture(ID3D11ShaderResourceView* srv);

	// ワールド行列セット
	void SetWorldMatrix(const Math::Matrix& m)
	{
		m_cb0.Work().mW = m;
	}

	// UV Offsetセット
	void SetUVOffset(const Math::Vector2& offset)
	{
		m_cb0.Work().UVOffset = offset;
	}

	// UV Tilingセット
	void SetUVTiling(const Math::Vector2& tiling)
	{
		m_cb0.Work().UVTiling = tiling;
	}

	// アルファテストの閾値設定
	// ・alpha	… この値以下のアルファ値のピクセルは描画されなくなる
	void SetAlphaTestThreshold(float alpha)
	{
		m_cb0.Work().AlphaTestThreshold = alpha;
	}

	// Set系でセットしたデータを、実際に定数バッファへの書き込む
	void WriteToCB()
	{
		m_cb0.Write();
	}

	//================================================
	// 描画関係
	//================================================

	// このシェーダをデバイスへセット
	void SetToDevice();

	// 3D線描画
	void DrawLine(const Math::Vector3& p1, const Math::Vector3& p2, const Math::Color& color);

	// 3D線描画
	void DrawVertices(const std::vector<Vertex>& vertices, D3D_PRIMITIVE_TOPOLOGY topology);

	// メッシュ描画
	// ・mesh			… 描画するメッシュ
	// ・materials		… 使用する材質配列
	void DrawMesh(const Mesh* mesh, const std::vector<Material>& materials);

	//================================================
	// 
	//================================================

	// 初期化
	bool Init();

	// 解放
	void Release();

	// 
	~EffectShader()
	{
		Release();
	}


private:

	// 3Dモデル用シェーダ
	ID3D11VertexShader*	m_VS = nullptr;				// 頂点シェーダー
	ID3D11InputLayout*	m_inputLayout = nullptr;		// 頂点入力レイアウト
	ID3D11InputLayout*	m_inputLayout_Model = nullptr;	// Model用頂点入力レイアウト

	ID3D11PixelShader*	m_PS = nullptr;				// ピクセルシェーダー

	// 定数バッファ
	//  ※定数バッファは、パッキング規則というものを厳守しなければならない
	//  <パッキング規則> 参考：https://docs.microsoft.com/ja-jp/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules
	//  ・構造体のサイズは16バイトの倍数にする。
	//  ・各項目(変数)は、16バイト境界をまたがないようにする。

	// 定数バッファ(オブジェクト単位更新)
	struct cbObject
	{
		Math::Matrix		mW;		// ワールド行列　行列は16バイトx4バイトの64バイトなのでピッタリ。

		Math::Vector2		UVOffset = { 0, 0 };
		Math::Vector2		UVTiling = { 1, 1 };

		// アルファテストの閾値
		float				AlphaTestThreshold = 0;
		float				tmp[3] = {};
	};
	ConstantBuffer<cbObject>	m_cb0;
};