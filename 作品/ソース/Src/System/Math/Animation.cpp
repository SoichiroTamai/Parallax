#include "Animation.h"

// 二分探索で、指定時間から次の配列要素のKeyIndexを求める関数
// list		… キー配列
// time		… 時間
// 戻り値	… 次の配列要素のIndex
template<class T>
int BinaySearchNextAnimKey(const std::vector<T>& list, float time)
{
	int low = 0;
	int high = (int)list.size();
	while (low < high)
	{
		int mid = (low + high) / 2;
		float midTime = list[mid].m_time;

		if (midTime <= time) low = mid + 1;
		else high = mid;
	}
	return low;
}

void AnimationData::Node::Interpolate(Matrix& rDst, float time)
{
	// クォータニオンによる回転補間
	Matrix rotate;
	Quaternion resultQuat;
	if (InterpolateRotations(resultQuat, time))
	{
		rotate.CreatFormQuaternion(resultQuat);
	}

	// ベクターによる座標補間
	Matrix	trans;
	Vec3		resultVec;
	if (InterpolateTranslations(resultVec, time))
	{
		trans.CreateTranslation(resultVec);
	}

	rDst = rotate * trans;
}

bool AnimationData::Node::InterpolateTranslations(Vec3& result, float time)
{
	if (m_translations.size() == 0) return false;

	// キー位置検索
	UINT keyIndex = BinaySearchNextAnimKey(m_translations, time);

	// 先頭のキーなら、先頭のデータを返す
	if (keyIndex == 0) {
		result = m_translations.front().m_vec;
		return true;
	}
	// 配列外のキーなら、最後のデータを返す
	else if (keyIndex >= m_translations.size()) {
		result = m_translations.back().m_vec;
		return true;
	}
	// それ以外（中間の時間なら）、その時間の値を補間計算で求める
	else {
		auto& prev = m_translations[keyIndex - 1];	// 前のキー
		auto& next = m_translations[keyIndex];		// 次のキー
		// 前のキーと次のキーの時間から、０～１間の時間を求める
		float f = (time - prev.m_time) / (next.m_time - prev.m_time);
		// 補間
		result = DirectX::XMVectorLerp(
			prev.m_vec,
			next.m_vec,
			f
		);
	}

	return true;
}

bool AnimationData::Node::InterpolateRotations(Quaternion& result, float time)
{
	if (m_rotations.size() == 0) return false;

	// キー位置検索
	UINT keyIndex = BinaySearchNextAnimKey(m_rotations, time);

	// 先頭のキーなら、先頭のデータを返す
	if (keyIndex == 0) {
		result = m_rotations.front().m_quat;
	}
	// 配列外のキーなら、最後のデータを返す
	else if (keyIndex >= m_rotations.size()) {
		result = m_rotations.back().m_quat;
	}
	// それ以外（中間の時間なら）、その時間の値を補間計算で求める
	else {
		auto& prev = m_rotations[keyIndex - 1];	// 前のキー
		auto& next = m_rotations[keyIndex];		// 次のキー
		// 前のキーと次のキーの時間から、０～１間の時間を求める
		float f = (time - prev.m_time) / (next.m_time - prev.m_time);
		// 補間
		result = DirectX::XMQuaternionSlerp(
			prev.m_quat,
			next.m_quat,
			f
		);
	}

	return true;
}

void Animator::AdvanceTime(std::vector<ModelInfo::Node>& rNodes, float speed)
{
	if (!m_spAnimation) { return; }

	// 全てのアニメーションノード ( モデルの行列を補間する情報 ) の行列補間を実行する
	for (auto& rAnimNode : m_spAnimation->m_nodes)
	{
		// 対応するモデルノードのインデックス
		UINT idx = rAnimNode.m_nodeOffset;

		// 対応するモデルノードの行列補間を実行
		rAnimNode.Interpolate(rNodes[idx].m_localTransform, m_time);
	}

	// アニメーションのフレームを１フレーム進める
	m_time += speed;

	// アニメーションデータの最後のフレームを超えたら
	if (m_time >= m_spAnimation->m_maxLength)
	{
		if (m_isLoop)
		{
			// アニメーションの最初に戻る (ループさせる)
			m_time = 0.0f;
		}
		else
		{
			m_time = m_spAnimation->m_maxLength;
		}
	}
}