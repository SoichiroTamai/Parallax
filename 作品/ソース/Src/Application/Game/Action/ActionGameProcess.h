#pragma once

#include "../GameProcess.h"

// ゲームシーンの管理と画面遷移を担当するクラス

// GameObject → GameProess → ActionGameProcess
class ActionGameProcess : public GameProcess
{
public:
	ActionGameProcess() {};				// コンストラク
	virtual ~ActionGameProcess() {};	// デストラクタ

	// 2D描画用
	void Deserialize(const json11::Json& jsonObj) override;
	void Draw2D() override;

	void Update() override;

private:

	// テクスチャ情報の格納用
	std::shared_ptr<Texture> m_DefaultCenterTex;
	std::shared_ptr<Texture> m_HandTex;
};