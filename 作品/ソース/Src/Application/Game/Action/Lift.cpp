#include "Lift.h"

void Lift::Deserialize(const json11::Json& jsonObj)
{
	GameObject::Deserialize(jsonObj);

	// 最初の地点を覚える
	m_mStart = m_mWorld;

	// 移動先情報の読み込み
	if (jsonObj["MoveTo"].is_array())
	{
		auto& p = jsonObj["MoveTo"].array_items();
		m_vGoal = Vec3(p[0].number_value(), p[1].number_value(), p[2].number_value());
	}

	// 移動スピードの読み込み
	if (jsonObj["Speed"].number_value())
	{
		m_speed = jsonObj["Speed"].number_value();
	}
}

void Lift::Update()
{
	// 動く前の行列を覚える
	GameObject::Update();

	// スタート地点とゴール地点の座標を取得
	auto& vStart = m_mStart.GetTranslation();
	auto& vGoal = m_vGoal;

	// 目標地点までのベクトル
	Vec3 vTo = vGoal - vStart;

	// 進行具合を加味して現在の地点を作り出す
	Vec3 vNow = vStart + vTo * m_progress;

	// 頂点を中間に合わせる
	m_mWorld.SetTranslation(vNow);

	// 進行具合の更新
	if (m_goTo)
	{
		// 向かっている時
		m_progress += m_speed;
		if (m_progress >= 1.0f)
		{
			m_goTo = false;
			m_progress = 1.0f;
		}
	}
	else
	{
		// 引き返している時
		m_progress -= m_speed;
		if (m_progress <= 0.0f)
		{
			m_goTo = true;
			m_progress = 0.0f;

		}
	}
}