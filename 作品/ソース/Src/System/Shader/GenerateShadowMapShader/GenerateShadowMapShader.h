#pragma once

//============================================================
//
// シャドウマップ(深度マップ)生成描画シェーダー
//
//============================================================
class GenerateShadowMapShader
{
public:

	// 平行光用深度マップ取得
	std::shared_ptr<Texture>	GetDirShadowMap() const { return m_dirLightShadowMap; }

	// ワールド行列セット
	void SetWorldMatrix(const Math::Matrix& m)
	{
		m_cb0.Work().mW = m;
	}

	//================================================
	// 描画
	//================================================
	// シャドウマップ生成描画開始
	void Begin();
	// シャドウマップ生成描画終了
	void End();

	// このシェーダをデバイスへセット
	void SetToDevice();

	// メッシュ描画
	// ・mesh			… 描画するメッシュ
	// ・materials		… 使用する材質配列
	void DrawMeshDepth(const Mesh* mesh, const std::vector<Material>& materials);

	//================================================
	// 初期化・解放
	//================================================

	// 初期化
	bool Init();
	// 解放
	void Release();
	// 
	~GenerateShadowMapShader()
	{
		Release();
	}

private:

	// シェーダー
	ID3D11VertexShader* m_VS = nullptr;			// 頂点シェーダー
	ID3D11InputLayout* m_inputLayout = nullptr;	// 頂点入力レイアウト

	ID3D11PixelShader* m_PS = nullptr;			// ピクセルシェーダー

	// 定数バッファ
	struct cbObject
	{
		Matrix mW;			// ワールド行列
		Matrix mLightVP;		// ライトカメラのビュー行列 * 射影行列
	};
	ConstantBuffer<cbObject>	m_cb0;

	// 深度マップ
	std::shared_ptr<Texture>	m_dirLightShadowMap;			// 平行光用 深度マップ
	std::shared_ptr<Texture>	m_dirLightZBuffer;				// 平行光用 Zバッファ


	//通常描画のRenderTargetとZバッファとViewportを記憶する変数
	ID3D11RenderTargetView* m_saveRT	= nullptr;
	ID3D11DepthStencilView* m_saveZ		= nullptr;
	UINT					m_numVP		= 1;
	D3D11_VIEWPORT			m_saveVP	= {};

};