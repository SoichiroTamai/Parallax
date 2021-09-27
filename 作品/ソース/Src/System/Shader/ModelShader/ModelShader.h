#pragma once

//============================================================
//
// モデル描画シェーダ
//
//============================================================
class ModelShader
{
public:

	//================================================
	// 設定・取得
	//================================================

	// ワールド行列セット
	void SetWorldMatrix(const Math::Matrix& m)
	{
		m_cb0.Work().mW = m;
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
	// 輪郭描画 (アウトライン)
	//================================================
	// 輪郭描画としてデバイスへセット
	void SetToDevice_Outline();
	// １つのメッシュを描画
	void DrawMesh_Outline(const Mesh* mesh);

	//================================================
	// 初期化・解放
	//================================================

	// 初期化
	bool Init();
	// 解放
	void Release();
	// 
	~ModelShader()
	{
		Release();
	}

private:

	// 3Dモデル用シェーダ
	ID3D11VertexShader* m_VS = nullptr;				// 頂点シェーダー
	ID3D11InputLayout*	m_inputLayout = nullptr;	// 頂点入力レイアウト

	ID3D11PixelShader*	m_PS = nullptr;				// ピクセルシェーダー

	// 輪郭描画(アウトライン)用シェーダ
	ID3D11VertexShader*	m_outlineVS = nullptr;		// 頂点シェーダー
	ID3D11PixelShader*	m_outlinePS = nullptr;		// ピクセルシェーダー

	// 定数バッファ(オブジェクト単位更新)
	struct cbObject
	{
		Math::Matrix		mW;		// ワールド行列　行列は16バイトx4バイトの64バイトなのでピッタリ。
	};
	ConstantBuffer<cbObject>	m_cb0;


	// 定数バッファ(マテリアル単位更新)
	struct cbMaterial
	{
		Math::Vector4	BaseColor;
		float			Metallic;
		float			Roughness;
		float			tmp[2];
	};
	ConstantBuffer<cbMaterial>	m_cb1_Material;
};