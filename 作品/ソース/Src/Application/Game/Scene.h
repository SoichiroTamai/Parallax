#pragma once

// バージョンテスト

// 前方宣言
class EditorCamera;
class GameObject;
class CameraComponent;
//class Application;

#include"../main.h"

class Scene
{
public:

	// シングルトンパターン		1つしかインスタンス化インスタンス化しない
	static Scene& GetInstance()
	{
		static Scene instance;		// 中身はstaticのみ
		return instance;
	}

	~Scene();	// デストラクタ

	void Init();	// 初期化
	void Deserialize();

	void RequestChangeScene(const std::string& fileName);	// シーン変更のリクエストを受付

	void Release();	// 解放
	void Update();	// 更新
	void Draw();	// 描画

	void AddObject(std::shared_ptr<GameObject> spObject);	// 渡されたミサイルを追加

	// 指定された名前で検索をかけて合致した最初のオブジェクトを返す
	std::shared_ptr<GameObject> FindObjectWithName(const std::string& name);

	// GameObjectのリストを返す
	const std::list<std::shared_ptr<GameObject>> GetObjects()const { return m_spObjects; }

	inline void SetTargetCamera(std::shared_ptr<CameraComponent> spCamera) { m_wpTargetCamera = spCamera; }

	void ImGuiUpdate();	// ImGuiの更新

	// デバックライン描画
	void AddDebugLine(const Math::Vector3& p1, const Math::Vector3& p2, const Math::Color& color = { 1,1,1,1 });

	// デバックスフィア描画
	void AddDebugSphereLine(const Math::Vector3& pos, float radius, const Math::Color& color = { 1,1,1,1 });

	// デバッグ軸描画
	void AddDebugCoordinateAxisLine(const Math::Vector3& pos, float scale = 1.0f);

	// 選択したオブジェクトを返す
	std::weak_ptr<GameObject> GetSelectObject(){ return m_wpImguiSelectObj;}
	void SetMoveObject(std::weak_ptr<GameObject> MoveObj){ m_wpImguiSelectObj = MoveObj;}

	// 画面中央のレイ情報を取得
	RayResult GetCenterRayResult() { return m_CenterRayResult; }

	// 2つ目に当たったオブジェクトのレイ情報を取得
	RayResult Get2ndRayResult() { return m_2ndRayResult; }

	float Get2ndDistance_First() { return m_2ndDistance_First; }
	
	// ウィンドウハンドルを取得
	HWND GetWndHandle_Secne() { return Application::GetInstance().m_window.GetWndHandle(); }

	bool Fls_Cursor = false;	// カーソルの表示の有無

	// 中心にあるオブジェクトを取得
	std::weak_ptr<GameObject> GetCenterObject() { return m_wpCenterObj;}

	bool GetEditorCameraEnable() { return m_editorCameraEnable; }

	// プレイヤー座標
	Matrix GetPlayerMatrix() { return m_PlayerMatrix; }
	//void   SetPlayerMatrix( Matrix mat ) { m_PlayerMatrix = mat; }

	void LoadJsonData(const std::string& sceneFilename);	// Jsonデータの読み込み

	bool ObjectMoveFlg	 = false;	// 長押し防止用 (true = オブジェクト移動中)
	bool ObjectCreateFlg = false;	// 長押し防止用 (オブジェクト生成)

	int GettNowSwitchData() { return NowSwitchData; }
	void SetNowSwitchData(int SwitchNo) { NowSwitchData += SwitchNo; }

	bool GetFlg_ImguiDraw() { return Flg_ImguiDraw; }

	//bool Flg_ObjMove = false;		// 元あったフラグ
	bool Flg_ObjMoveable = false;	// オブジェクトの移動が可能かどうか (true = 可能)

	// 2D座標から3D座標へ変換
	Vec3 ConvertScreenToWorld(int sx, int sy, float fZ, int screen_w, int screen_h, const Matrix& rView, const Matrix& rPrj);
	
	// ターゲットカメラの中心座標から指定した距離先の3D座標を取得
	Vec3 GetConvertScreenCenterToWorld(float fZ);

	Vec3 ConvertWorldToScreen(Vec3 v, int screen_w, int screen_h, const Matrix& rView, const Matrix& rPrj);

	Vec3 GetConvertWorldToScreenCenter(Vec3 v);

private:

	Scene();	// コンストラクタ		シングルトンパターン	NEWなど出来なくなる

	void LoadObjectDatas(const std::string& sceneFilename);
	void LoadScene(const std::string& SceneFilename, const std::string& GameObjectFilename);

	void ExecChangeScene();					// シーンを実際に変更するところ
	void Reset();							// シーンをまたぐ時にリセットする処理

	std::string m_nextSceneFileName = "";	// 次のシーンのJsonファイル名
	bool m_isRequestChangescene = false;	// シーン遷移のリクエストがあったか

	std::shared_ptr<ModelInfo>		m_spSky = nullptr;		// スカイスフィア
	std::shared_ptr<EditorCamera>	m_spCamera = nullptr;	// エディターカメラ

	bool	m_editorCameraEnable = false;					// 視点切り替え用フラグ ( false = FPS , trur = TPS )

	std::list<std::shared_ptr<GameObject>> m_spObjects;		// 配列の順番がバラバラ <シェアードポインタ>

	// Imguiで選択されたオブジェクト 及び 中心のレイにあるオブジェクト
	std::weak_ptr<GameObject>	m_wpImguiSelectObj;

	// ターゲットのカメラ (弱参照 → 参照先を見るだけのポインタ)
	std::weak_ptr<CameraComponent> m_wpTargetCamera;

	// デバックライン描画用頂点配列
	std::vector<EffectShader::Vertex> m_debugLines;

	void ImGuiPrefabFactoryUpdate();
	//void LoadJsonData(const std::string& sceneFilename);	// Jsonデータの読み込み
	void CreateObject(json11::Json& jsonData);				// オブジェクト作成 ( 指定したJson内に"ClassName"があれば作成可 )

	// 入力したJsonファイル名の格納用
	std::string JsonFileName = "";

	//std::shared_ptr<ImGuiHelper::ImGuiLogWindo>	m_Editor_Log;
	ImGuiHelper::ImGuiLogWindo	m_Editor_Log;

	//float RayDistance = 0.0f;
	RayResult m_CenterRayResult;	// 画面中央のレイ情報
	RayResult m_2ndRayResult;		// 2つ目に当たったオブジェクト (選択したオブジェクトの移動及び拡縮に使用)

	float m_2ndDistance_First = 0.0f;	// オブジェクトの拡縮に使用する基準の距離 (この変数 - 現在の距離 で拡大率を作成)
	bool  Flg_2ndDistance = false;

	// クラグ
	bool LookFlg = false;
	bool LookFlgF1 = false;
	bool MoveNotFlg = false;

	int frame = 0;
	std::weak_ptr<GameObject>	m_wpCenterObj;

	bool Flg_ImguiDraw = false;	// true = Imguiを表示	
	bool LookFlgF2 = false;
	
	Matrix m_PlayerMatrix;		// プレイヤーの座標

	UINT NowSwitchData = 0;

	std::string nowStage_FileName = "";		// 現在のステージ

	// シーン別の更新処理 <-------------------------------
	void Stage_GimmickUpdate();		// ギミックステージ(オブジェクト生成ギミックを使ったステージ)
	// --------------------------------------------------->

	// 画面中央のレイ情報を検索
	void CenterRay();
	
	// 画面中央のレイで2番目に当たるもの
	void CenterRay2nd();
	
	// シェーダー <-----------------------------------------------------
	//-------------------------
	// 平行光
	//-------------------------
	Vec3	m_lightDir = { 0,-1,0 };	//方向
	Vec3	m_lightColor = { 1,1,1 };	//光の色
	float	m_lightBrightess = 1.0f;	//光の輝度 (明るさ)

	//-------------------------
	// 点光
	//-------------------------
	float	m_PointlightBrightess = 1.0f;	//光の輝度 (明るさ)

	// ----------------------------------------------------------------->

	// サウンド <-------------------------------------------------------
	std::unique_ptr<DirectX::AudioEngine>			m_audioEng = nullptr;	// サウンドエンジン (サウンド全体の管理)
	
	std::vector	<std::unique_ptr<DirectX::SoundEffect>>			m_soundEffects;		// 再生するサウンド１つの元データ
	std::list	<std::unique_ptr<DirectX::SoundEffectInstance>>	m_instances;		// サウンドの再生インスタンス(鳴らす度に生成)
	
	bool m_canPlaySE = false;
	// ----------------------------------------------------------------->
};