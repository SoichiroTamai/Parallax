#pragma once

// アニメーションキー（クォータニオン）
struct AnimKeyQuaternion
{
	float			m_time = 0;		// 時間
	Quaternion	m_quat;			// クォータニオン
};

// アニメーションキー（ベクトル）
struct AnimKeyVector3
{
	float			m_time = 0;		// 時間
	Vec3			m_vec;			// 3Dベクトルデータ
};

//=======================
// アニメーションデータ
//=======================
struct AnimationData
{
	// アニメーション名
	std::string		m_name;
	// アニメの長さ
	float			m_maxLength = 0;

	// １ノードのアニメーションデータ
	struct Node
	{
		int		m_nodeOffset = -1;	// 対象モデルノードのOffset値

		// 各チャンネル
		std::vector<AnimKeyVector3>		m_translations;	// 位置キーリスト
		std::vector<AnimKeyQuaternion>	m_rotations;	// 回転キーリスト

		void	Interpolate(Matrix& rDst, float time);
		bool	InterpolateTranslations(Vec3& result, float time);
		bool	InterpolateRotations(Quaternion& result, float time);
	};

	// 全ノード用アニメーションデータ
	std::vector<Node>	m_nodes;
};

class Animator
{
public:
	inline void SetAnimation(std::shared_ptr<AnimationData>& rData, bool isLoop = true)
	{
		m_spAnimation = rData;

		m_time = 0.0f;
	}

	// アニメーションが終了している？
	bool IsAnimatorEnd() const
	{
		if (m_spAnimation == nullptr) { return true; }
		if (m_time >= m_spAnimation->m_maxLength) { return true; }

		return false;
	}

	// アニメーションの更新
	void AdvanceTime(std::vector<ModelInfo::Node>& rNodes, float speed = 1.0f);

private:
	std::shared_ptr<AnimationData> m_spAnimation = nullptr;	// 再生するアニメーションデータ

	float	m_time = 0.0f;
	bool	m_isLoop = true;	// デフォルト値 ( true = ループする )
};