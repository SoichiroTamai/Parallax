#pragma once

#include "GameObject.h"

// ゲームシーンの管理者をカテゴリ分けするために、GamePropcessにGameObjectを継承させる
class GameProcess : public GameObject
{
public:
	GameProcess() {};
	virtual ~GameProcess() {};

};