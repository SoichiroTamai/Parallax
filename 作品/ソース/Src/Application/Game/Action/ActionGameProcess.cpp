#include "ActionGameProcess.h"
#include "../Scene.h"

void ActionGameProcess::Deserialize(const json11::Json& jsonObj)
{
	// UI用テクスチャの読み込み
	m_DefaultCenterTex = ResFac.GetTexture("Data/Texture/UI.png");			// 通常
	m_HandTex = ResFac.GetTexture("Data/Texture/cursor_hand_icon.png");		// 手
}

void ActionGameProcess::Draw2D()
{
	if (!m_DefaultCenterTex) { return; }
	if (Scene::GetInstance().GetEditorCameraEnable()) { return; }

	// 2D描画
	//m_mWorld.SetTranslation(Vec3(-100, 0, 0));	// 左に移動
	SHADER.m_spriteShader.SetMatrix(m_mWorld);		// 行列を渡す  m_mWorld … 単位行列
	
	if (Scene::GetInstance().GetCenterRayResult().m_hit)
	{

		auto centerObj = Scene::GetInstance().GetCenterObject().lock();

		//if (centerObj == nullptr) {
		//	SHADER.m_spriteShader.DrawTex(m_DefaultCenterTex.get(), 0, 0);		// .get()　… ポインターを取得
		//	return;
		//}

		if (centerObj->GetTag() & TAG_MoveObject )
			SHADER.m_spriteShader.DrawTex(m_HandTex.get(), 0, 0);				// 手
		else
			SHADER.m_spriteShader.DrawTex(m_DefaultCenterTex.get(), 0, 0);		// ・
	}
	else {
		SHADER.m_spriteShader.DrawTex(m_DefaultCenterTex.get(), 0, 0);		// ・
	}
}

void ActionGameProcess::Update()
{
	//if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	//{
	//	// 現在のシーンを調べる (シーンの切り替え)
	//	Scene::GetInstance().RequestChangeScene("Data/Scene/ShootingGame.json");
	//}
}