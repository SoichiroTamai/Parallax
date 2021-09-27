#include "main.h"

#include"Game/Scene.h"

//===================================================================
// メイン
//===================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
	// メモリリークを知らせる
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// COM初期化
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	// mbstowcs_s関数で日本語対応にするために呼ぶ
	setlocale(LC_ALL, "japanese");

	//===================================================================
	// 実行
	//===================================================================
	APP.Execute();

	// COM解放
	CoUninitialize();

	return 0;
}

// アプリケーション初期設定
bool Application::Init(int w, int h)
{

	//===================================================================
	// ウィンドウ作成
	//===================================================================
	if (m_window.Create(w, h, "Parallax", "Window") == false) {
		MessageBoxA(nullptr, "ウィンドウ作成に失敗", "エラー", MB_OK);
		return false;
	}

	//===================================================================
	// フルスクリーン確認
	//===================================================================
	bool bFullScreen = false;
	if (MessageBoxA(m_window.GetWndHandle(), "フルスクリーンにしますか？", "確認", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
		bFullScreen = true;
	}

	//===================================================================
	// Direct3D初期化
	//===================================================================

	// デバイスのデバッグモードを有効にする
	bool deviceDebugMode = false;
	#ifdef _DEBUG
	deviceDebugMode = true;
	#endif

	// Direct3D初期化
	std::string errorMsg;
	if (D3D.Init(m_window.GetWndHandle(), w, h, deviceDebugMode, errorMsg) == false) {
		MessageBoxA(m_window.GetWndHandle(), errorMsg.c_str(), "Direct3D初期化失敗", MB_OK | MB_ICONSTOP);
		return false;
	}

	// フルスクリーン設定
	if (bFullScreen) {
		D3D.GetSwapChain()->SetFullscreenState(TRUE, 0);
	}

	// シェーダー初期化
	SHADER.Init();

	//===================================================================
	// オーディオ初期化
	//===================================================================

	//===================================================================
	// imgui 初期化
	//===================================================================

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsClassic();

	ImGui_ImplWin32_Init(m_window.GetWndHandle());
	ImGui_ImplDX11_Init(D3D.GetDev(), D3D.GetDevContext());


	// 日本語化 -------------------------------------------------------------------------------------------
	#include"imgui/ja_glyph_ranges.h"

	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig config;
	config.MergeMode = true;
	io.Fonts->AddFontDefault();

	// 日本語対応
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msgothic.ttc", 13.0f, &config, glyphRangesJapanese);
	// ----------------------------------------------------------------------------------------------------

	return true;
}

// アプリケーション終了
void Application::Release()
{
	// imgui 解放
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// シェーダ解放
	SHADER.Release();

	// Direct3D解放
	D3D.Release();

	// オーディオ解放


	// ウィンドウ削除
	m_window.Release();

}

// アプリケーション実行
void Application::Execute()
{
	//===================================================================
	// 初期設定(ウィンドウ作成、Direct3D初期化など)
	//===================================================================
	if (APP.Init(1280, 720) == false) {
		return;
	}

	// シーンの初期化
	Scene::GetInstance().Init();

	//===================================================================
	// ゲームループ
	//===================================================================

	// 時間
	DWORD baseTime = timeGetTime();
	int count = 0;

	// ループ
	while (1)
	{

		// 処理開始時間Get
		DWORD st = timeGetTime();

		// ゲーム終了指定があるときはループ終了
		if (m_endFlag)
		{ 
			break;
		}

		//=========================================
		//
		// ウィンドウ関係の処理
		//
		//=========================================

		// ウィンドウのメッセージを処理する
		m_window.ProcessMessage();

		// ウィンドウが破棄されてるならループ終了
		if (m_window.IsCreated() == false)
		{
			break;
		}

		//=========================================
		//
		// オーディオ処理
		//
		//=========================================


		//=========================================
		//
		// ゲーム処理
		//
		//=========================================

		//Imgui開始
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// ImGui Demo ウィンドウ表示　※すごく参考になるウィンドウ	ImGui_demo,cpp 参照
		//ImGui::ShowDemoWindow(nullptr);

		Scene::GetInstance().ImGuiUpdate();

		// バックバッファクリア
		D3D.GetDevContext()->ClearRenderTargetView(D3D.GetBackBuffer()->GetRTView(),
			Math::Color(0.0f, 0.0f, 1.0f, 1.0f));	// 青色で塗りつぶす


		// Zバックバッファクリア ( 3Dの前後判定に使用 )
		D3D.GetDevContext()->ClearDepthStencilView(D3D.GetZBuffer()->GetDSView(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);		// 削除時のフラグ設定


		// シーンの更新
		Scene::GetInstance().Update();
		
		// シーンの描画
		Scene::GetInstance().Draw();

		// ImGui描画実行
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// バックバッファを画面表示
		D3D.GetSwapChain()->Present(0, 0);


		//=========================================
		//
		// フレームレート制御
		//
		//=========================================

		constexpr int kSecond = 1000;				// １秒のミリ秒
		constexpr float kFpsRefreshFrame = 500;		// FPS計測のタイミング

		// 処理終了時間Get
		DWORD et = timeGetTime();
		// Fps制御
		DWORD ms = kSecond / m_maxFps;
		if (et - st < ms)
		{
			Sleep(ms - (et - st));	// 速すぎたら待つ
		}

		// FPS計測
		count++;
		if (st - baseTime >= kFpsRefreshFrame)
		{
			m_fps = (count * kSecond) / (st - baseTime);
			baseTime = st;
			count = 0;
		}

		// ESCで強制終了
		if (GetAsyncKeyState(VK_ESCAPE) && 0x8000)
			break;
	}

	Scene::GetInstance().Release();

	//===================================================================
	// アプリケーション解放
	//===================================================================
	Release();
}