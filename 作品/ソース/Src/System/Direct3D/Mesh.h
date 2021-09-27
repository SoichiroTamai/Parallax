#pragma once

//==========================================================
// メッシュ用 頂点情報
//==========================================================
struct MeshVertex
{
	Math::Vector3	Pos;		// 座標
	Math::Vector2	UV;			// UV
	Math::Vector3	Normal;		// 法線
	unsigned int	Color = 0xFFFFFFFF;			// RGBA色(容量削減のため、各色0～255のUINT型)

	Math::Vector3	Tangent;	// 接線
};

//==========================================================
// メッシュ用 面情報
//==========================================================
struct MeshFace
{
	UINT Idx[3];				// 三角形を構成する頂点のIndex
};

//==========================================================
// メッシュ用 サブセット情報
//==========================================================
struct MeshSubset
{
	UINT		MaterialNo = 0;		// マテリアルNo

	UINT		FaceStart = 0;		// 面Index　このマテリアルで使用されている最初の面のIndex
	UINT		FaceCount = 0;		// 面数　FaceStartから、何枚の面が使用されているかの
};

//==========================================================
//
// メッシュクラス
//
//==========================================================
class Mesh
{
public:

	//=================================================
	// 取得・設定
	//=================================================

	// サブセット情報配列を取得
	const std::vector<MeshSubset>&		GetSubsets() const { return m_subsets; }

	// 頂点の座標配列を取得
	const std::vector<Math::Vector3>&	GetVertexPositions() const { return m_positions; }
	// 面の配列を取得
	const std::vector<MeshFace>&		GetFaces() const { return m_faces; }

	// 軸平行境界ボックス取得
	const DirectX::BoundingBox&			GetBoundingBox() const { return m_aabb; }
	// 境界球取得
	const DirectX::BoundingSphere&		GetBoundingSphere() const { return m_bs; }

	// メッシュデータをデバイスへセットする
	void SetToDevice() const;


	//=================================================
	// 作成・解放
	//=================================================

	// メッシュ作成
	// ・vertices		… 頂点配列
	// ・faces			… 面インデックス情報配列
	// ・subsets		… サブセット情報配列
	// 戻り値			… 成功：true
	bool Create(const std::vector<MeshVertex>& vertices, const std::vector<MeshFace>& faces, const std::vector<MeshSubset>& subsets);

	// 解放
	void Release()
	{
		m_vb.Release();
		m_ib.Release();
		m_subsets.clear();
		m_positions.clear();
		m_faces.clear();
	}

	~Mesh()
	{
		Release();
	}

	//=================================================
	// 処理
	//=================================================

	// 指定サブセットを描画
	void DrawSubset(int subsetNo) const;

	// 
	Mesh() {}

private:

	// 頂点バッファ
	Buffer						m_vb;
	// インデックスバッファ
	Buffer						m_ib;

	// サブセット情報
	std::vector<MeshSubset>		m_subsets;

	// 境界データ
	DirectX::BoundingBox		m_aabb;	// 軸平行境界ボックス
	DirectX::BoundingSphere		m_bs;	// 境界球

	// 座標のみの配列(複製)
	std::vector<Math::Vector3>	m_positions;
	// 面情報のみの配列(複製)
	std::vector<MeshFace>		m_faces;

private:
	// コピー禁止用
	Mesh(const Mesh& src) = delete;
	void operator=(const Mesh& src) = delete;
};

//==========================================================
// マテリアル(glTFベースのPBRマテリアル)
//==========================================================
struct Material
{
	//---------------------------------------
	// 材質データ
	//---------------------------------------

	// 名前
	std::string					Name;

	// 基本色
	std::shared_ptr<Texture>	BaseColorTex;				// 基本色テクスチャ
	Math::Vector4				BaseColor = { 1,1,1,1 };	// 基本色のスケーリング係数(RGBA)

	// 金属性、粗さ
	std::shared_ptr<Texture>	MetallicRoughnessTex;		// B:金属製 G:粗さ
	float						Metallic = 0.0f;			// 金属性のスケーリング係数
	float						Roughness = 1.0f;			// 粗さのスケーリング係数

	// 自己発光
	std::shared_ptr<Texture>	EmissiveTex;				// 自己発光テクスチャ
	Math::Vector3				Emissive = { 0,0,0 };		// 自己発光のスケーリング係数(RGB)

	// 法線マップ
	std::shared_ptr<Texture>	NormalTex;					// 法線マップテクスチャ

	Material()
	{
		BaseColorTex			= D3D.GetWhiteTex();
		MetallicRoughnessTex	= D3D.GetWhiteTex();
		EmissiveTex				= D3D.GetWhiteTex();
		NormalTex				= D3D.GetNormalTex();
	}
};
