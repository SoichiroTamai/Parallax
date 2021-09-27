#pragma once

#include "../GameObject.h"

class Scene;
class Gimmick;

class Human : public GameObject
{
public:
	virtual void Deserialize(const json11::Json& jsonObj) override;
	virtual void Update() override;
	void Draw()	override;	// 描画

	//着地しているかどうか
	bool IsGround() { return m_isGround; }

	bool IsEditorCameraEnable() { return m_editorCameraEnable; }

	// アニメーションをセット
	void SetAnimation(const char* pAnimName, bool isLoop = true);

private:
	void UpdateMove();	// 操作・キャラの行動による移動

	float	m_moveSpeed = 0.1f;	// キャラの移動速度
	Vec3	m_pos;				// ワールド行列上の座標

	void UpdateCamera();		// 操作によるカメラの回転と移動
	float m_camRotSpeed = 0.2f;	// カメラの回転速度

	void UpdateRotate(const Vec3& rMoveDir);		// 操作やキャラクターの行動による回転計算
	float m_rotateAngle = 10.0f;					// キャラクターの回転角度
	Vec3	m_rot;									// ワールド行列上の回転角度

	//float m_gravity = 0.01f;		// 重力の強さ
	float m_jumpPow = 0.2f;			// ジャンプの強さ
	float m_PowerSpringJump = 1.6f;	// ばねジャンプ時の倍率 = m_jump_ * m_PowerSpringJump
	//Vec3  m_force ;				// キャラクターにかかる移動させる力 (落下、跳躍、移動)

	static const float s_allowToStepHeight;		// 歩いて乗り越えられる段差の高さ
	static const float s_landingHeight;			// 地面から足が離れていても着地していると判定する高さ (坂道などを下るときに宙に浮くのを避ける)

	/*
		static const int	→ ここで初期化可能
		static const float	→ ここでは初期化出来ない
	*/

	virtual void UpdateCollision() override;						// 当たり判定全般

	virtual bool CheckGround(float& rDstDistance) override;		// 地面との判定
	virtual void CheckBump() override;							// ぶつかり判定 (横)

	Vec3	m_prevPos;							// 1フレーム前の座標
	bool	m_isGround;							// 着地しているかどうか	

	void	MoveObject();					// オブジェクトを動かす
	void	CreateObject_ScreenCenter();	// 画面中央にオブジェクトを生成

	Matrix	ObjRot{};						// オブジェクトの回転量

	Matrix	ObjMat{};						// オブジェクトの座標
	Matrix	ObjMovePos{};					// オブジェクトの移動先
	Matrix	ObjScale{};						// オブジェクトの拡大率

	bool m_editorCameraEnable = false;		// カメラ切り替え用

	bool	DiffFlg = false;
	float	StandardScale = 1.0f;

	Vec3 moveVec;

	// FPSカメラの注視点の位置修正
	Vec3 FPS_LookatPoint_Correction = Vec3( 0.0f, 1.3f, 0.0f );

	// スイッチ
	std::weak_ptr<Gimmick>	m_wpSwitch;

	// ギミックのタグ
	int Hit_TAG = GIMMICK_TAG::TAG_None;

	// 初期座標
	Vec3 FirstPos = Vec3();

	//	オブジェクト操作中か (true = 操作中) 
	bool Obj_MoveFlg = false;

	// オブジェクトを動かし終わった
	bool Obj_MoveEndFlg = false;

	// オブジェクトの生成が可能か (true = 可能)
	bool Obj_GeneratableFlg = false;

	// 移動中のオブジェクト
	std::shared_ptr<GameObject> m_spMoveObj = nullptr;

	// アニメーション <------------------------------------
	void ChangeStand();		// 待機状態に遷移
	void ChangeMove();		// 移動状態に遷移
	void ChangeJump();		// ジャンプ状態に遷移	

	bool IsChangeMove();	//移動状態に遷移するかどうか
	bool IsChangeJump();	//ジャンプ状態に遷移するかどうか
	
	bool CanJump() { return IsGround(); }

	Animator m_animator;

	class ActionBase
	{
	public:
		virtual void Update(Human& rOwner) = 0;
	};

	class ActionWait : public ActionBase
	{
	public:
		void Update(Human& rOwner) override;
	};

	class ActionWalk : public ActionBase
	{
	public:
		void Update(Human& rOwner) override;
	};

	class ActionJamp : public ActionBase
	{
	public:
		void Update(Human& rOwner) override;
	};

	std::shared_ptr<ActionBase> m_spActionState = nullptr;
	// --------------------------------------------------->
};