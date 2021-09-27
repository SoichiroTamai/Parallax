#pragma once

class Texture;

// サンプラステート
enum class SamplerFilterMode {
	Point,			// ポイントフィルタ
	Linear,			// 線形フィルタ
	Anisotropic,	// 異方性フィルタ
};

enum class SamplerAddressingMode {
	Wrap,			// ラップモード
	Clamp,			// クランプモード
};

// ブレンドモード用定数
enum class BlendMode {
	NoBlend,		// ブレンドをしない
	Alpha,			// 半透明ブレンド
	Add				// 加算ブレンド
};

//=======================================================================================
//
// Direct3Dクラス
//
//	Direct3D11に必要な変数や、よく行う操作をまとめています。
//
//
//=======================================================================================
class Direct3D {
public:

	//==============================================================
	//
	// 取得
	//
	//==============================================================

	// Direct3Dデバイス取得
	ID3D11Device*				GetDev() { return m_pDevice; }

	// Direct3Dデバイスコンテキスト取得
	ID3D11DeviceContext*		GetDevContext() { return m_pDeviceContext; }

	// SwapChain取得
	IDXGISwapChain*				GetSwapChain() { return m_pGISwapChain; }

	// バックバッファ取得
	std::shared_ptr<Texture>	GetBackBuffer() { return m_backBuffer; }

	// Zバッファ取得
	std::shared_ptr<Texture>	GetZBuffer() { return m_zBuffer; }

	// 1x1 白テクスチャ取得
	std::shared_ptr<Texture>	GetWhiteTex() const { return m_texWhite; }

	// 1x1 Z向き法線マップ取得
	std::shared_ptr<Texture>	GetNormalTex() const { return m_texNormal; }

	// ビューポート取得
	void						GetViewport(Math::Viewport& out) const;


	//==============================================================
	//
	// 初期化・解放
	//
	//==============================================================

	// 初期化
	// ・hWnd			… ウィンドウのハンドル
	// ・w				… X解像度
	// ・h				… Y解像度
	// ・debugDevice	… デバッグモード
	// ・errMsg			… (出力)エラーメッセージが入ってくる
	bool Init(HWND hWnd, int w, int h, bool deviceDebug, std::string& errMsg);

	// 解放
	void Release();

	//==============================================================
	//
	// パイプラインステート作成関数
	//
	//==============================================================

	// 深度ステンシルステート作成
	// ・device				… D3Dデバイス
	// ・zEnable			… 深度テスト有効
	// ・zWriteEnable		… 深度書き込み有効
	ID3D11DepthStencilState* CreateDepthStencilState(bool zEnable, bool zWriteEnable) const;

	// ラスタライザーステート作成
	// ・device				… D3Dデバイス
	// ・cullMode			… カリングモード
	// ・fillMode			… 三角形の描画モード
	// ・depthClipEnable	… Zクリッピングを有効にする
	// ・scissorEnable		… 切り抜き範囲を有効にする
	ID3D11RasterizerState* CreateRasterizerState(D3D11_CULL_MODE cullMode, D3D11_FILL_MODE fillMode, bool depthClipEnable, bool scissorEnable) const;

	// サンプラーステート作成
	// ・device				… D3Dデバイス
	// ・filterType			… 0:補間なし 1:線形フィルタ 2:異方性フィルタ
	// ・maxAnisotropy		… 異方性フィルタ時の最大値　2, 4, 6, 8, 10, 12, 14, 16 のいずれか
	// ・addressingMode		… テクスチャアドレッシングモード 0:Wrap 1:Clamp
	// ・comparisonModel	… 比較モードON　シャドウマッピングなどで使用する
	ID3D11SamplerState* CreateSamplerState(SamplerFilterMode filterType, UINT maxAnisotropy, SamplerAddressingMode addressingMode, bool comparisonModel) const;

	// ブレンドステート作成
	// ・device				… D3Dデバイス
	// ・mode				… 合成モード
	ID3D11BlendState* CreateBlendState(BlendMode mode) const;

	//==============================================================

	// 頂点を描画する簡易的な関数
	// ・topology		… 頂点をどのような形状で描画するか　D3D_PRIMITIVE_TOPOLOGYマクロを使用
	// ・vertexCount	… 頂点数
	// ・pVertexStream	… 頂点配列の先頭アドレス
	// ・stride			… １頂点のバイトサイズ
	void DrawVertices(D3D_PRIMITIVE_TOPOLOGY topology, int vertexCount, const void* pVertexStream, UINT stride);

	//==============================================================

	// 
	~Direct3D() {
		Release();
	}

private:

	ID3D11Device*				m_pDevice = nullptr;			// Direct3D11の中心になるクラス。全体の管理とバッファ、シェーダ、テクスチャなどのリソース作成などを行う。D3D9とは違って、このクラスは描画関係のメンバ関数を持たない。
	ID3D11DeviceContext*		m_pDeviceContext = nullptr;		// 描画処理を行うクラス。
																// 内部的には、レンダリングコマンドと呼ばれるバイナリデータを作成し、GPUに送る。GPUがレンダリングコマンドを解析することで実際の描画処理が行われる。
	IDXGISwapChain*				m_pGISwapChain = nullptr;		// ウインドウへの表示やダブルバッファリングなどを行うクラス。マルチサンプリング、リフレッシュレートなどの設定もできるみたい。

	// バッファ
	std::shared_ptr<Texture>	m_backBuffer;					// バックバッファ
	std::shared_ptr<Texture>	m_zBuffer;						// Zバッファ

	// DrawVertices用 頂点バッファ
	Buffer					m_tempFixedVertexBuffer[10];	// 固定長 頂点バッファ
	Buffer					m_tempVertexBuffer;				// 可変長 頂点バッファ

	//------------------------
	// 便利テクスチャ
	//------------------------
	// 1x1 白テクスチャ
	std::shared_ptr<Texture>	m_texWhite;

	// 1x1 Z向き法線マップ
	std::shared_ptr<Texture>	m_texNormal;

//-------------------------------
// シングルトン
//-------------------------------
private:

	Direct3D() {
	}

public:
	static Direct3D &GetInstance() {
		static Direct3D instance;
		return instance;
	}
};

// 簡単にアクセスできるようにするためのマクロ
#define D3D Direct3D::GetInstance()
