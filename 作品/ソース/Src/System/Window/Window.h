#pragma once

//=====================================================
//
// ウィンドウ
//
//=====================================================
class Window {
public:

	//===================================
	// 取得・設定
	//===================================

	// ウィンドウハンドル取得
	HWND GetWndHandle() const { return m_hWnd; }
	// ウィンドウが存在する？
	bool IsCreated() const { return m_hWnd ? true : false; }
	// マウスホイールの変化量を取得
	int GetMouseWheelVal() const { return m_mouseWheelVal; }
	// インスタンスハンドル取得
	HINSTANCE GetInstanceHandle() const { return GetModuleHandle(0); }

	// クライアントサイズの設定
	void SetClientSize(int w, int h);

	//===================================
	// 初期化・解放
	//===================================

	// ウィンドウ作成
	bool Create(int clientWidth, int clientHeight, const std::string& titleName, const std::string& windowClassName);

	// 解放
	void Release();

	//
	~Window() {
		Release();
	}

	//===================================
	// 処理
	//===================================

	// ウィンドウメッセージを処理する
	//  戻り値：終了メッセージが来たらfalseが返る
	bool ProcessMessage();

	// ファイルを開くダイアログボックスを表示
	// ・filepath		… 選択されたファイルパスが入る
	// ・title			… ウィンドウのタイトル文字
	// ・filters		… 指定された拡張子のみ表示されるようになる
	static bool OpenFileDialog(std::string& filepath, const std::string& title = "ファイルを開く", const char* filters = "全てのファイル\0*.*\0");
	// ファイル名をつけて保存ダイアログボックスを表示
	// ・filepath		… 選択されたファイルパスが入る
	// ・title			… ウィンドウのタイトル文字
	// ・filters		… 指定された拡張子のみ表示されるようになる
	// ・defExt			… ユーザーが拡張子を入力しなかった場合、これが自動的に付く
	static bool SaveFileDialog(std::string& filepath, const std::string& title = "ファイルを保存", const char* filters = "全てのファイル\0*.*\0", const std::string& defExt = "");

	// 画面サイズを取得
	float GetWindowWidth() { return WindowWidth; }
	float GetWindowHeight() { return WindowHeight; }

	// 画面のアスペクト比を取得
	float GetWindowAspectRatio() const { return WindowWidth / WindowHeight; }

private:

	// ウィンドウハンドル
	HWND	m_hWnd = nullptr;

	// マウスホイール値
	int		m_mouseWheelVal = 0;

	// ウィンドウ関数
	static LRESULT CALLBACK callWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	float WindowWidth = 0.0f;
	float WindowHeight = 0.0f;
};