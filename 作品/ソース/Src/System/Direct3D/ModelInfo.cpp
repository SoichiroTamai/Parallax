#include "ModelInfo.h"
#include "GLTFLoader.h"

// コンストラクタ
ModelInfo::ModelInfo()
{
}

// デストラクタ
ModelInfo::~ModelInfo()
{
	Release();
}

//  ロード ( 読み込み ) ※ノード対応 ver
bool ModelInfo::Load(const std::string& filename)
{
	// ファイルの完全パスを取得
	std::string fileDir = GetDirFromPath(filename);

	//GLTFの読み込み
	std::shared_ptr<GLTFModel> spGltfModel = LoadGLTFModel(filename);
	if (spGltfModel == nullptr) { return false; }

	// ノード格納場所のメモリ確保
	m_originalNodes.resize(spGltfModel->Nodes.size());

	for (UINT i = 0; i < spGltfModel->Nodes.size(); ++i)
	{
		// 入力元ノード
		const GLTFNode& rSrcNode = spGltfModel->Nodes[i];

		// 出力先ノード
		Node& rDestNode = m_originalNodes[i];

		// ノード情報のセット
		rDestNode.m_name = rSrcNode.Name;
		rDestNode.m_localTransform = rSrcNode.LocalTransform;

		// ノードの内容がメッシュだったら
		if (rSrcNode.IsMesh)
		{
			// リストにメッシュのインスタンス化
			rDestNode.m_spMesh = std::make_shared<Mesh>();
			
			// メッシュ情報の作成
			if (rDestNode.m_spMesh)
			{
				//									頂点情報配列		面情報配列			サブセット情報配列
				rDestNode.m_spMesh->Create(rSrcNode.Mesh.Vertices, rSrcNode.Mesh.Faces, rSrcNode.Mesh.Subsets);
			}
		}
	}

	// マテリアル配列を受け取れるサイズのメモリを確保
	m_materials.resize(spGltfModel->Materials.size());

	for (UINT i = 0; i < m_materials.size(); ++i)
	{
		//src = source の略
		//dst = destinationの略
		const GLTFMaterial& rSrcMaterial = spGltfModel->Materials[i];
		Material& rDstMaterial = m_materials[i];
	
		// 名前
		rDstMaterial.Name = rSrcMaterial.Name;

		// 基本色
		rDstMaterial.BaseColor = rSrcMaterial.BaseColor;
		rDstMaterial.BaseColorTex = std::make_shared<Texture>();

		rDstMaterial.BaseColorTex = ResFac.GetTexture(fileDir + rSrcMaterial.BaseColorTexture);
		if (rDstMaterial.BaseColorTex == nullptr)
		{
			// 失敗したら白い1ピクセルのテクスチャを格納
			rDstMaterial.BaseColorTex = D3D.GetWhiteTex();
		}

		// 金属性・粗さ
		rDstMaterial.Metallic = rSrcMaterial.Metallic;
		rDstMaterial.Roughness = rSrcMaterial.Roughness;
		rDstMaterial.MetallicRoughnessTex = std::make_shared<Texture>();
		if (rDstMaterial.MetallicRoughnessTex->Load(fileDir + rSrcMaterial.MetallicRoughnessTexture) == false)
		{
			// 読み込めなかった場合は、代わりに白画像を使用
			rDstMaterial.MetallicRoughnessTex = D3D.GetWhiteTex();
		}

		// 法線マップ
		rDstMaterial.NormalTex = std::make_shared<Texture>();
		if (rDstMaterial.NormalTex->Load(fileDir + rSrcMaterial.NormalTexture) == false)
		{
			// 読み込めなかった場合は、代わりにZ向き法線マップを使用
			rDstMaterial.NormalTex = D3D.GetNormalTex();
		}
	}

	// アニメーションデータ
	m_spAnimationos.resize(spGltfModel->Animations.size());

	for (UINT i = 0; i < m_spAnimationos.size(); ++i)
	{
		const GLTFAnimationData& rSrcAnimation = *spGltfModel->Animations[i];
		
		m_spAnimationos[i] = std::make_shared<AnimationData>();
		AnimationData& rDstAnimation = *(m_spAnimationos[i]);

		rDstAnimation.m_name = rSrcAnimation.m_name;

		rDstAnimation.m_maxLength = rSrcAnimation.m_nodes.size();

		rDstAnimation.m_nodes.resize(rSrcAnimation.m_nodes.size());

		for (UINT i = 0; i < rDstAnimation.m_nodes.size(); ++i)
		{
			rDstAnimation.m_nodes[i].m_nodeOffset = rSrcAnimation.m_nodes[i]->m_nodeOffset;
			rDstAnimation.m_nodes[i].m_translations = rSrcAnimation.m_nodes[i]->m_translations;
			rDstAnimation.m_nodes[i].m_rotations = rSrcAnimation.m_nodes[i]->m_rotations;
		}
	}

	return true;
}

const std::shared_ptr<AnimationData> ModelInfo::GetAnimation(const std::string& animName) const
{
	for (auto&& anim : m_spAnimationos)
	{
		if (anim->m_name == animName)
		{
			return anim;
		}
	}

	return nullptr;;
}

// リリース
void ModelInfo::Release()
{
	m_materials.clear();
	m_originalNodes.clear();
}