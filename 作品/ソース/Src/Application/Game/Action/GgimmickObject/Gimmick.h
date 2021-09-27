#pragma once
#include "../../GameObject.h"


class Gimmick : public GameObject
{
public:
	virtual void Deserialize(const json11::Json& jsonObj) override;
	virtual void Update() override;

private:

	struct Gimmick_HitData
	{
		bool HitFlg = false;
		std::shared_ptr<GameObject> HitObj = nullptr;
	};

	Gimmick_HitData GimmickTopResult;
	Gimmick_HitData GimmickCannonResult;

	// 上にオブジェクトがあるか 
	bool CheckObject_on_Top();

	bool CheckObject_Hit();

	bool CheckCannon();
	void CannonUpdate();

	int Gimmick_Fps = 0;	// ギミック用 フレームレート
};