#pragma once

//#include"../Game/GameObject.h"
class GameObject;

//=================================
// キー番号定数
//=================================
namespace Input
{
	enum Axes  // axis の複数形
	{
		L,
		R,
		AXS_MAX
	};

	enum Buttons
	{
		A,
		B,
		X,
		Y,
		L1,
		R1,
		L2,
		R2,
		L3,
		R3,
		BIN_MAX
	};

	// ボタン配置
	namespace ButtonsVirtualKeyCode
	{
		// 基本移動
		inline int KC_Jump		= VK_SPACE;		// A
		
		// オブジェクトを移動
		inline int KC_ObjMove	= VK_LBUTTON;	// L2

		// オブジェクト生成
		inline int KC_CreateObj = VK_LBUTTON;	// L2
		inline int KC_CreateBox		= '1';		// Y
		inline int KC_PowerSpring	= '2';		// B
		inline int KC_MoveObj		= '3';		// X
	}

	// ボタン配置
	namespace OperationButtons
	{
		inline Buttons Button_Jump		  = A;		// ジャンプ
		
		inline Buttons Button_ObjMove	  = L2;		// オブジェクトを移動

		inline Buttons Button_CreateObj	  = L2;		// オブジェクト生成
		inline Buttons Button_CreateBox	  = Y;		// 生成するオブジェクトを変更(Box)
		inline Buttons Button_PowerSpring = X;		// 生成するオブジェクトを変更(ばね)
		inline Buttons Button_MoveObj	  = B;		// 生成するオブジェクトを変更(移動オブジェクト)

	}
}

//=================================
// 入力コンポーネントクラス
//=================================
class InputComponent
{
public:

	// ボタン状態
	enum
	{
		FREE	= 0x00000000,
		ENTER	= 0x00000001,
		STAY	= 0x00000002,
		EXIT	= 0x00000004,
	};

	// コンストラクター：オーナーの設定・ボタンの初期化
	InputComponent(GameObject& owner);
	
	virtual ~InputComponent() {};

	// 入力の更新
	virtual void Update() {};

	// 操作軸取得
	inline const Math::Vector2& GetAxis(Input::Axes no) const
	{
		assert(no != Input::Axes::AXS_MAX);
		return m_axes[no];
	}

	// ボタンフラグ取得
	inline int GetButtoon(Input::Buttons no) const
	{
		assert(no != Input::Buttons::BIN_MAX);
		return m_buttons[no];
	}

	// ボタンを押した
	void PushButton(Input::Buttons no);
	// ボタンを離す
	void ReleaceButton(Input::Buttons no);

protected:

	// 操作軸
	std::array<Math::Vector2, Input::Axes::AXS_MAX> m_axes;
	// 操作軸
	std::array<int, Input::Buttons::BIN_MAX> m_buttons;
	// 持ち主
	GameObject& m_owner;

	void KeySettings(int KeyCode, Input::Buttons no);
};

//=================================
// キーボード用入力コンポーネント (プレイヤー)
//=================================
class PlayerInputComponent: public InputComponent
{
public:
	PlayerInputComponent(GameObject& owner) :InputComponent(owner) {}

	virtual void Update() override;
};

//=================================
// 敵用入力コンポーネント
//=================================
class EnemyInputComponent : public InputComponent
{
public:
	EnemyInputComponent(GameObject& owner) :InputComponent(owner) {}

	virtual void Update() override;

	float rotate = 0.0f;
};

//=================================
// 地面を動き回るキャラクタ用のInputコンポーネント
// WASD：地面
// マウス：カメラ回転
//=================================
class ActionPlayerInputComponent : public InputComponent
{
public:
	ActionPlayerInputComponent(GameObject& rOwner) : InputComponent(rOwner){}

	virtual void Update() override;

	POINT m_prevMousePos;
};

//=================================
// LessonGame用のInputコンポーネント
// WASD：地面
// WA (前進後退) , AD (左右旋回)
// マウス：カメラ回転
//=================================
class LessonGameInputComponent : public InputComponent
{
public:
	LessonGameInputComponent(GameObject& rOwner) : InputComponent(rOwner) {}

	virtual void Update() override;

	POINT m_prevMousePos;
};