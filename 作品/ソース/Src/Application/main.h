#pragma once


//============================================================
// アプリケーションクラス
//	APP.～ でどこからでもアクセス可能
//============================================================
class Application
{
// メンバ
public:

	// アプリケーション実行
	void Execute();

	// アプリケーション終了
	void End() { m_endFlag = true; }


	//=====================================================
	// グローバルデータ
	//=====================================================

	// ゲームウィンドウクラス
	Window		m_window;

	// FPS制御
	int				m_fps = 0;					// 現在のFPS値
	int				m_maxFps = 60;				// 最大FPS


private:

	// アプリケーション初期化
	bool Init(int w, int h);

	// アプリケーション解放
	void Release();

	// ゲーム終了フラグ trueで終了する
	bool		m_endFlag = false;


//=====================================================
// シングルトンパターン
//=====================================================
private:
	// 
	Application() {}

public:
	static Application &GetInstance(){
		static Application Instance;
		return Instance;
	}
};

// 簡単にアクセスできるようにするためのマクロ
#define APP Application::GetInstance()
