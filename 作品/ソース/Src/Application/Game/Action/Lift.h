#include "../GameObject.h"

class Lift : public GameObject
{
public:
	virtual void Deserialize(const json11::Json& jsonObj) override;
	virtual void Update() override;

private:

	// スタート地点
	Matrix m_mStart;

	// ゴール地点
	Vec3 m_vGoal = {};

	// リフトが進むスピード
	float m_speed = 0.0f;

	// ゴール地点に向かっている場合 (0-1)
	float m_progress = 0.0f;

	// ゴール地点に向かっている (true)、引き返えしてきている状態 (false)
	bool m_goTo = true;
};