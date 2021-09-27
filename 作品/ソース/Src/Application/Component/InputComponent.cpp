#include"InputComponent.h"
#include"../Game/Scene.h"

// コンストラクター：オーナーの設定・ボタンの初期化
InputComponent::InputComponent(GameObject& owner):m_owner(owner)
{
	// 操作軸初期化
	for (auto& axis : m_axes)
	{
		axis = { 0.0f,0.0f };
	}

	m_buttons.fill(FREE);	// 全要素 FREEに
}

void InputComponent::PushButton(Input::Buttons no)
{
	assert(no != Input::Buttons::BIN_MAX);

	// 押している
	if (m_buttons[no] & STAY)
	{
		m_buttons[no] &= ~ENTER;	// ENTER反転とAND = ENTERだけをOFF
	}
	// 押していない
	else
	{
		m_buttons[no] |= ENTER;	// ENTERをOR	= ENTERをON
		m_buttons[no] |= STAY;	// STAYをOR		= STAYをON
	}
}

void InputComponent::ReleaceButton(Input::Buttons no)
{
	assert(no != Input::Buttons::BIN_MAX);

	// 押している
	if (m_buttons[no] & STAY)
	{
		m_buttons[no] &= ~ENTER;	// ENTER反転とAND = ENTERだけをOFF
		m_buttons[no] &= ~STAY;		// STAY反転とAND = STAYだけをOFF
		m_buttons[no] |= EXIT;		// EXUTをOR		=　EXUTをON
	}
	// 押していない
	else
	{
		m_buttons[no] &= ~EXIT;		// EXIT反転とAND = EXITだけをOFF
	}
}



void PlayerInputComponent::Update()
{
	// 操作軸初期化
	for (auto& axis : m_axes)
	{
		axis = { 0.0f,0.0f };
	}

	// [左の軸値] 入力処理
	if (GetAsyncKeyState(VK_UP) & 0x8000)	{ m_axes[Input::Axes::L].y = 1.0f; }
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)	{ m_axes[Input::Axes::L].y = -1.0f; }
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000){ m_axes[Input::Axes::L].x = 1.0f; }
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)	{ m_axes[Input::Axes::L].x = -1.0f; }

	// [右の軸値] 入力処理
	if (GetAsyncKeyState('W') & 0x8000) { m_axes[Input::Axes::R].y = 1.0f; }
	if (GetAsyncKeyState('S') & 0x8000) { m_axes[Input::Axes::R].y = -1.0f; }
	if (GetAsyncKeyState('D') & 0x8000) { m_axes[Input::Axes::R].x = 1.0f; }
	if (GetAsyncKeyState('A') & 0x8000) { m_axes[Input::Axes::R].x = -1.0f; }

	// [ボタン] 入力処理
	if (GetAsyncKeyState('Z')) { PushButton(Input::Buttons::A); }
	else { ReleaceButton(Input::Buttons::A); }

	if (GetAsyncKeyState('X')) { PushButton(Input::Buttons::B); }
	else { ReleaceButton(Input::Buttons::B); }

	if (GetAsyncKeyState('C')) { PushButton(Input::Buttons::X); }
	else { ReleaceButton(Input::Buttons::X); }

	if (GetAsyncKeyState('V')) { PushButton(Input::Buttons::Y); }
	else { ReleaceButton(Input::Buttons::Y); }

	if (GetAsyncKeyState('Q')) { PushButton(Input::Buttons::L1); }
	else { ReleaceButton(Input::Buttons::L1); }

	if (GetAsyncKeyState('E')) { PushButton(Input::Buttons::R1); }
	else { ReleaceButton(Input::Buttons::R1); }

}

void EnemyInputComponent::Update()
{
	//m_axes[Input::Axes::L].y = 1.0f;
	//m_axes[Input::Axes::R].y = 1.0f;

	//rotate += 2.0f/270.0f;	// 2/n° … 最大角度 

	//m_axes[Input::Axes::L].y = 1.0f;
	//m_axes[Input::Axes::R].y = sin(rotate);
	//m_axes[Input::Axes::R].x = sin(rotate);
}

void ActionPlayerInputComponent::Update()
{
	// 各軸の初期化
	for (auto& rAxis : m_axes)
	{
		rAxis = { 0.0f,0.0f };
	} 
	
	// 左軸の入力 (移動用)
	if (GetAsyncKeyState('W')) { m_axes[Input::Axes::L].y =  1.0f; }
	if (GetAsyncKeyState('S')) { m_axes[Input::Axes::L].y = -1.0f; }
	if (GetAsyncKeyState('D')) { m_axes[Input::Axes::L].x =  1.0f; }
	if (GetAsyncKeyState('A')) { m_axes[Input::Axes::L].x = -1.0f; }

	// 右軸の入力(マウスの入力)
	POINT nowMousePos;
	GetCursorPos(&nowMousePos);	// マウスの現在位置の取得
	m_axes[Input::R].x = (float)(nowMousePos.x - m_prevMousePos.x);		// 今の座標 - 1つ前の座標
	m_axes[Input::R].y = (float)(nowMousePos.y - m_prevMousePos.y);		//			〃
	
	// カーソルを固定 (Ingui表示中)
	if (Scene::GetInstance().GetFlg_ImguiDraw())
	{
		ShowCursor(true);
	}
	else
	{
		SetCursorPos(APP.m_window.GetWindowWidth() / 2.0f, APP.m_window.GetWindowHeight() / 2.0f);
		GetCursorPos(&nowMousePos);

		// カーソル非表示
		ShowCursor(false);

		//// エディターカメラ切り替え用
		//if (GetAsyncKeyState(VK_F3))
		//	Scene::GetInstance().Fls_Cursor = Scene::GetInstance().Fls_Cursor;
	}

	m_prevMousePos = nowMousePos;	// 今回のマウスの位置を変える

	// ボタン入力 (キー位置) <--------------------------------------------------
	KeySettings(Input::ButtonsVirtualKeyCode::KC_Jump, Input::OperationButtons::Button_Jump);

	KeySettings(VK_F5, Input::Buttons::R3);

	//オブジェクト生成
	KeySettings(Input::ButtonsVirtualKeyCode::KC_CreateObj, Input::OperationButtons::Button_CreateObj);

	// 生成するオブジェクト
	KeySettings(Input::ButtonsVirtualKeyCode::KC_CreateBox	, Input::OperationButtons::Button_CreateBox);
	KeySettings(Input::ButtonsVirtualKeyCode::KC_PowerSpring, Input::OperationButtons::Button_PowerSpring);
	KeySettings(Input::ButtonsVirtualKeyCode::KC_MoveObj	, Input::OperationButtons::Button_MoveObj);

	// ------------------------------------------------------------------------->
}

void LessonGameInputComponent::Update()
{
	//// 各軸の初期化
	//for (auto& rAxis : m_axes)
	//{
	//	rAxis = { 0.0f,0.0f };
	//}

	//// 左軸の入力
	//if (GetAsyncKeyState('W')) { m_axes[Input::Axes::L].y = 1.0f; }
	//if (GetAsyncKeyState('S')) { m_axes[Input::Axes::L].y = -1.0f; }
	//if (GetAsyncKeyState('D')) { m_axes[Input::Axes::R].x = 1.0f; }
	//if (GetAsyncKeyState('A')) { m_axes[Input::Axes::R].x = -1.0f; }

	//// 右軸の入力(マウスの入力入力)
	//POINT nowMousePos;
	//GetCursorPos(&nowMousePos);	// マウスの現在位置の取得
	//m_axes[Input::R].x = (float)(nowMousePos.x - m_prevMousePos.x);		// 今の座標 - 1つ前の座標
	//m_axes[Input::R].y = (float)(nowMousePos.y - m_prevMousePos.y);		//			〃
	//m_prevMousePos = nowMousePos;	// 今回のマウスの位置を変える
}


// 押している間ON
void InputComponent::KeySettings( int KeyCode, Input::Buttons no )
{
	//if (GetAsyncKeyState(KeyCode)) { PushButton(no); }
	//else { ReleaceButton(no); }

	static bool flg;

	if (GetAsyncKeyState(KeyCode) && !flg) { PushButton(no); flg = true; }
	else { ReleaceButton(no); flg = false; }
}