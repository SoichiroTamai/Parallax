// ===============================
// ログウィンドウクラス
// ===============================
namespace ImGuiHelper {
	class ImGuiLogWindo {
	public:
		// ログをクリア
		void Clear() { m_Buf.clear(); };

		// 文字列を追加
		template<class... Args>	// printfのような記述ができる
		void AddLog(const std::string& fmt, Args... args)
		{
			// 改行を加える
			std::string str = fmt + "\n";
			// 第2引数以降で指定された変数第１引数の文字列内に組み込む
			m_Buf.appendf(str.c_str(), args...);
			// Log を追加した時は一番下までスクロールするようにフラグを立てる
			m_ScrollToBottom = true;
		}

		// ウィンドウ画面
		void ImguiUpdate(const char* title, bool* p_opened = NULL)
		{
			ImGui::Begin(title, p_opened);
			// ウィンドウの幅に関係がない書式で文字を描画する
			ImGui::TextUnformatted(m_Buf.begin());
			if (m_ScrollToBottom) {
				// 1番したまでスクロール
				ImGui::SetScrollHere(1.0f);
			}
			m_ScrollToBottom = false;
			ImGui::End();
		}

	private:
		// ログウィンドウに表示する文字を格納する変数
		ImGuiTextBuffer		m_Buf;
		bool				m_ScrollToBottom = false;	// ダーティフラグ
	};
}