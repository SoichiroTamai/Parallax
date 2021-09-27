#pragma once

#include"../Game/GameObject.h"

// ===========================
// モデルコンポーネント
// ===========================

class ModelComponent : public std::enable_shared_from_this<ModelComponent>
{
public:
	ModelComponent(GameObject& owner) : m_owner(owner) {}

	// 有効フラグ
	bool IsEnable()const { return m_enable; }
	void SetEnable(bool enable) { m_enable = enable; }

	// メッシュ取得
	inline const std::shared_ptr<Mesh> GetMesh(UINT index)const
	{
		if (index >= m_coppiedNodes.size()) { return nullptr; }
		return m_coppiedNodes[index].m_spMesh;
	}

	// 文字列を元にノードの検索
	inline ModelInfo::Node* FindNode(const std::string& name)
	{
		// コピーしたノードの中から検索をかける
		for (auto&& node: m_coppiedNodes)
		{
			if (node.m_name == name)
			{
				return &node;
			}
		}
		return nullptr;
	}

	//アニメーションデータ取得
	const std::shared_ptr<AnimationData> GetAnimation(const std::string& animName) const
	{
		if (!m_spModel) { return nullptr; }
		return m_spModel->GetAnimation(animName);
	}

	// ノード取得
	const std::vector<ModelInfo::Node>& GetNodes() const { return m_coppiedNodes; }

	// ノード取得 (変更可能)
	std::vector<ModelInfo::Node>& GetChangeableNoeds() { return m_coppiedNodes; }

	// モデル取得
	inline const std::shared_ptr<ModelInfo>& GetModel() const { return m_spModel; }

	// モデルセット
	void SetModedl(const std::shared_ptr<ModelInfo>& rModel);

	// StanderdShaderで描画
	void Draw();

	// シャドウマップ生成描画
	void DrawShadowMap();

private:

	std::vector<ModelInfo::Node> m_coppiedNodes;	// 個別管理のため、オリジナルからコピーして保持する配列

	// 有効
	bool m_enable = true;

	// モデルデータの参照
	std::shared_ptr<ModelInfo> m_spModel;

	GameObject& m_owner;
};