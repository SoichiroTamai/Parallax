#include"Scene.h"
#include"GameObject.h"
#include"../Component/CameraComponent.h"

#include"EditorCamera.h"

#include"Action/Human.h"	

//#include"../main.h"

// コンストラクタ
Scene::Scene()
{
}

// デコンストラクタ
Scene::~Scene()
{

}

// 初期化
void Scene::Init()
{
	m_spCamera = std::make_shared<EditorCamera>();

	m_spSky = ResouceFactory::GetInstance().GetModel("Data/Sky/Sky.gltf");

	Deserialize();

	//// 爆発エフェクト(確認用)
	//std::shared_ptr<AnimationEffect> spExp = std::make_shared<AnimationEffect>();
	//spExp->SetAnimationInfo(KdResFac.GetTexture("Data/Texture/Explosion00.png"), 10.0f, 5, 5, 0.0f);
	//AddObject(spExp);

	// サウンド <-------------------------------------------------------
	// サウンドエンジンの準備
	DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_EnvironmentalReverb | DirectX::AudioEngine_ReverbUseFilters;

	// サウンドエンジンの作成
	m_audioEng = std::make_unique<DirectX::AudioEngine>(eflags);
	m_audioEng->SetReverb(DirectX::Reverb_Default);

	//サウンドの読み込み
	if (m_audioEng != nullptr)
	{
		try
		{
			// wstringに変換 (const char* は受けっとてくれない)
			std::wstring wFilename = sjis_to_wide("Data/Audio/BGM/Castle.wav");
			
			// 読み込み
			m_soundEffects.push_back( std::make_unique<DirectX::SoundEffect>(m_audioEng.get(), wFilename.c_str()) );
		
			// SEの読み込み
			wFilename = sjis_to_wide("Data/Audio/SE/ItemGet.wav");
			m_soundEffects.push_back( std::make_unique<DirectX::SoundEffect>(m_audioEng.get(), wFilename.c_str()) );
		}
		catch (...)
		{
			assert(0 && "Sound File Load Error");
		}
	}

	//サウンドの再生
	if (m_soundEffects[0] != nullptr)
	{
		// 再生オプション
		DirectX::SOUND_EFFECT_INSTANCE_FLAGS flags = DirectX::SoundEffectInstance_Default;

		// サウンドエフェクトからサウンドインスタンスの作成 (1度のみの再生する)
		auto instance = (m_soundEffects[0]->CreateInstance(flags));

		// サウンドインスタンスを使って作成
		if (instance)
		{
			//instance->Play();
		}
		m_instances.push_back(std::move(instance));
	}
	// ----------------------------------------------------------------->
}

void Scene::Deserialize()
{
	// 最初のシーンの読み込み
	LoadObjectDatas(SceneData::GamePlayScreen_FileName);	// 最初に読み込まれるシーン
	LoadObjectDatas(MapData::GameFirstStage_FileName);		// 最初にロードされるマップ

	nowStage_FileName = MapData::GameFirstStage_FileName;
}

// 解放
void Scene::Release()
{
	if (m_spCamera)
	{
		m_spCamera = nullptr;
	}

	// 配列の入れものを削除
	m_spObjects.clear();

	// サウンド後の処理
	m_audioEng = nullptr;
	m_instances.clear();
}

// 更新
void Scene::Update()
{
	//OutputDebugStringA("更新関数\n");

	// シェーダー (ポイントライト) < ----------------------
	// 点光の登録をリセットする
	SHADER.ResetPointLight();

	// ポイントライトを追加
		// ポイントライトを追加
	SHADER.AddPointLight({ 0.0f,4.0f, 0.0f }, 10, { 1.0f, 1.0f, 1.0f }, 3.0f);
	SHADER.AddPointLight({ -5.0f,4.0f,2.0f }, 10, { 1.0f, 1.0f, 1.0f }, 3.0f);
	SHADER.AddPointLight({ -7.0f,4.0f,-2.0f },15, { 1.0f, 1.0f, 1.0f }, 5.0f);
	SHADER.AddPointLight({ -10.0f,4.0f,-10.0f }, 5, { 1.0f, 1.0f, 1.0f }, 2.0f);

	//SHADER.AddPointLight({-10.0f,5.0f,-10.0f}, 30, { 1.0f, 1.0f, 1.0f }, 10.0f);
	//SHADER.AddPointLight({-10.0f,5.0f,10.0f}, 30, { 1.0f, 1.0f, 1.0f }, 10.0f);
	//SHADER.AddPointLight({10.0f,5.0f,-10.0f}, 30, { 1.0f, 1.0f, 1.0f }, 10.0f);
	//SHADER.AddPointLight({10.0f,5.0f,10.0f}, 30, { 1.0f, 1.0f, 1.0f }, 10.0f);
	
	// ---------------------------------------------------->

	if (m_editorCameraEnable)
	{
		m_spCamera->Update();
	}

	auto selectObject = m_wpImguiSelectObj.lock();

	for (auto spObj : m_spObjects)
	{
		if (spObj->GetTag() & TAG_Player) {
			m_PlayerMatrix = spObj->GetMatrix();
			//spObj->SetMatrix(m_PlayerMatrix);
		}

		// 選択したものがあれば (選択しているオブジェクトは更新しない)
		if (Flg_ImguiDraw) {
			if (spObj == selectObject) { continue; }
		}

		spObj->Update();
	}

	for (auto spObjectItr = m_spObjects.begin(); spObjectItr != m_spObjects.end();)
	{
		// 寿命が尽きていたらリストから除外
		if ((*spObjectItr)->IsAlive() == false)
		{
			spObjectItr = m_spObjects.erase(spObjectItr);
		}
		else
		{
			++spObjectItr;
		}
	}

	// シーン遷移のリクエストがあった場合、変更する
	if (m_isRequestChangescene)
	{
		ExecChangeScene();
	}

	// ステージごとの更新処理
	if (nowStage_FileName == MapData::GimmickStage_FileName)
		Stage_GimmickUpdate();

	// シェーダー <-----------------------------------------------------
	{
		if (Flg_ImguiDraw) {

			//疑似的な太陽の表示 (位置確認用)
			const Vec3 sunPos = { 0.f,5.f,0.f };
			Vec3 sunDir = m_lightDir;
			sunDir.Normalize();
			Vec3 color = m_lightColor;
			color.Normalize();
			Math::Color sunColor = color;
			sunColor.w = 1.0f;
			AddDebugLine(sunPos, sunPos + sunDir * 2, sunColor);
			AddDebugSphereLine(sunPos, 0.5f, sunColor);
		}
	}
	// ----------------------------------------------------------------->

	// サウンド <-------------------------------------------------------
	if (GetAsyncKeyState('Q'))
	{
		// SEが鳴らせる状態か
		if (m_canPlaySE)
		{
			// サウンドエフェクトからサウンドインスタンスの作成
			DirectX::SOUND_EFFECT_INSTANCE_FLAGS flags = DirectX::SoundEffectInstance_Default;

			auto instance = (m_soundEffects[1]->CreateInstance(flags));	// [0]:BGM　[1]:SE

			// 再生
			if (instance)
			{
				//instance->Play();
			}

			// 再生中インスタンスリストに加える
			m_instances.push_back(std::move(instance));

			m_canPlaySE = false;
		}
	}
	else
	{
		m_canPlaySE = true;
	}

	// サウンドエンジンの更新
	if (m_audioEng == nullptr)
	{
		m_audioEng->Update();
	}

	// 再生中ではないインスタンスは終了したとしてリストから削除
	for (auto iter = m_instances.begin(); iter != m_instances.end();)
	{
		if (iter->get()->GetState() != DirectX::SoundState::PLAYING)
		{
			// リストから削除
			iter = m_instances.erase(iter);

			continue;
		}

		++iter;
	}
	// ----------------------------------------------------------------->
}

// 描画
void Scene::Draw()
{
	if (m_editorCameraEnable)
	{
		m_spCamera->SetToShader();
	}
	else
	{
		// 消されないようにする
		std::shared_ptr<CameraComponent> spCamera = m_wpTargetCamera.lock();
		if (spCamera)
		{
			spCamera->SetToShader();
		}
	}

	//============================
	// シャドウマップ生成描画
	//============================
	SHADER.m_genShadowMapShader.Begin();

	// 全オブジェクトを描画
	for (auto& obj : m_spObjects)
	{
		obj->DrawShadowMap();
	}

	SHADER.m_genShadowMapShader.End();

	// ライトの情報を描画デバイスにセット
	SHADER.m_cb8_Light.Write();

	//// エフェクトシェーダを描画デバイスにセット
	SHADER.m_effectShader.SetToDevice();

	Math::Matrix skyScale = DirectX::XMMatrixScaling(100.0f, 100.0f, 100.0f);

	SHADER.m_effectShader.SetWorldMatrix(skyScale);

	// モデルの描画(メッシュ情報とマテリアルの情報を渡す)
	if (m_spSky)
	{
		//// エフェクトシェーダを描画デバイスにセット
		SHADER.m_effectShader.DrawMesh(m_spSky->GetMesh(0).get(), m_spSky->GetMaterials());
	}

	// 不透明物描画	
	// シャドウマップをセット
	D3D.GetDevContext()->PSSetShaderResources(102, 1, SHADER.m_genShadowMapShader.GetDirShadowMap()->GetSRViewAddress());

	//SHADER.m_standardShader.SetToDevice();	// デフォルト
	SHADER.m_modelShader.SetToDevice();			// モデルシェーダー

	for (auto pObject : m_spObjects)
	{
		pObject->Draw();
	}

	// 半透明物描画
	SHADER.m_effectShader.SetToDevice();
	SHADER.m_effectShader.SetTexture(D3D.GetWhiteTex()->GetSRView());

	for (auto spObj : m_spObjects)
	{
		spObj->DrawEffect();
	}

	////////////////////////////////////////
	// 2D 描画
	///////////////////////////////////////

	// 2D描画用のシェーダーを開始
	SHADER.m_spriteShader.Begin();

	// 全てのオブジェクトの2D描画を行う
	for (auto obj : m_spObjects)
	{
		obj->Draw2D();
	}

	// シェーダー終了
	SHADER.m_spriteShader.End();

	//////////////////////////////////////////////
	// デバックラインの描画
	/////////////////////////////////////////////

	// デバックラインの描画
	SHADER.m_effectShader.SetToDevice();										// 不透明描画 . 描画準備
	SHADER.m_effectShader.SetTexture(D3D.GetWhiteTex()->GetSRView());
	{
		// デバッグ用
		if (Flg_ImguiDraw)
		{
			AddDebugLine(Math::Vector3(), Math::Vector3(0.0f, 10.0f, 0.0f));

			AddDebugSphereLine(Math::Vector3(5.0f, 5.0f, 0.0f), 2.0f);

			AddDebugCoordinateAxisLine(Math::Vector3(5.0f, 5.0f, 5.0f), 3.0f);
		}

		// Zバッフ使用OFF・書き込みOFF
		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZDisable_ZWriteDisable, 0);

		// 点があれば
		if (m_debugLines.size() >= 1)
		{											// 単位行列
			SHADER.m_effectShader.SetWorldMatrix(Math::Matrix());					// 拡大率以外の行列
												// 描画する点の配列 
			SHADER.m_effectShader.DrawVertices(m_debugLines, D3D_PRIMITIVE_TOPOLOGY_LINELIST);		// 今回は線で描画する

			m_debugLines.clear();		// m_debugLinesは重いので使わないときは消しておく
		}

		// Zバッフ使用ON・書き込みON
		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZEisable_ZWriteEnable, 0);
	}
}

void Scene::Reset()
{
	m_spObjects.clear();			// メインのリストをクリア
	m_wpImguiSelectObj.reset();		// Imguiが選んでいるオブジェクトをクリア
	m_wpTargetCamera.reset();		// カメラのターゲットになってたいるキャラクタのリセット
	m_spSky = nullptr;				// 空のクリア
}

void Scene::LoadObjectDatas(const std::string& sceneFilename)
{
	// GameObjectリストを空にする
	//Reset();	// 各項目のクリア

		// JSON 読み込み
	json11::Json json = ResFac.GetJSON(sceneFilename);
	if (json.is_null())
	{
		assert(0 && "[LoadJson]jsonファイル読み込み失敗");
		return;
	}

	// オブジェクトリスト取得
	auto& objJsonSceneDataList = json["SceneObjects"].array_items();
	auto& objJsonStageDataList = json["StageObjects"].array_items();

	// オブジェクト生成ループ
	for (auto&& objJsonSceneData : objJsonSceneDataList)
	{
		CreateObject(objJsonSceneData);
	}

	// オブジェクト生成ループ
	for (auto&& objJsonStageData : objJsonStageDataList)
	{
		CreateObject(objJsonStageData);
	}
}

void Scene::LoadScene(const std::string& SceneFilename, const std::string& GameObjectFilename)
{
	// GameObjectリストを空にする
	Reset();	// 各項目のクリア

		// 最初のシーンの読み込み
	LoadObjectDatas(SceneFilename);			// 最初に読み込まれるシーン
	LoadObjectDatas(GameObjectFilename);	// 最初にロードされるマップ

}

// Jsonデータの読み込み
void Scene::LoadJsonData(const std::string& sceneFilename)
{
	// JSON 読み込み
	json11::Json json = ResFac.GetJSON(sceneFilename);
	if (json.is_null())
	{
		assert(0 && "[LoadJson]jsonファイル読み込み失敗");
		// ログウィンドウに記述
		m_Editor_Log.AddLog(sceneFilename + u8"生成失敗");
		return;
	}

	CreateObject(json);
}

// オブジェクトの作成
void Scene::CreateObject(json11::Json& jsonData)
{
	std::string jsondata = jsonData["ClassName"].string_value();

	// "ClassName" がなければ帰る
	if (jsondata == "") {

		if (JsonFileName == "")
			m_Editor_Log.AddLog(jsonData["Name"].string_value() + u8"　ClassName がありません。 ");
		else
			m_Editor_Log.AddLog(jsonData["Name"].string_value() + "(" + JsonFileName + ")" + u8"　ClassName がありません。");

		return;
	}

	// オブジェクト作成
	auto newGameObj = CreateGameObject(jsondata);

	// プレハブ指定ありの場合は、プレハブ側のものをベースにこのJSONをマージする
	MergePrefab(jsonData);

	// オブジェクトのデシリアライズ
	newGameObj->Deserialize(jsonData);

	// リストへ追加
	AddObject(newGameObj);

	if (JsonFileName == "")
		m_Editor_Log.AddLog(jsonData["Name"].string_value() + u8"　Prefab生成成功");
	else
		m_Editor_Log.AddLog(jsonData["Name"].string_value() + "(" + JsonFileName + ")" + u8"　Prefab生成成功");
}

void Scene::AddObject(std::shared_ptr<GameObject> spObject)
{
	if (spObject == nullptr) { return; }
	m_spObjects.push_back(spObject);
}

std::shared_ptr<GameObject> Scene::FindObjectWithName(const std::string& name)
{
	// 文字列比較
	for (auto&& obj : m_spObjects)
	{
		if (obj->GetName() == name) { return obj; }
	}

	// 見つからなかったらnullが帰る
	return nullptr;
}

void Scene::ImGuiUpdate()
{
	// エディターカメラがOFFの時は更新しない
	// if (!m_editorCameraEnable) { return; }

	// レイ情報初期化用
	RayResult RayResult_Initialization;

	// 画面中央の地点のオブジェクトを判別 <=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	////左クリックされたら
	//if (GetAsyncKeyState(VK_LBUTTON))
	//{
	//	if (!LookFlg) {
	//		LookFlg = true;
	//		Flg_ObjMove = !Flg_ObjMove;
	//	}
	//}
	//else
	//{
	//	LookFlg = false;
	//}

	// オブジェクトを選択不可能にする
	if (GetAsyncKeyState(VK_F1))
	{
		if (!LookFlgF1) {
			LookFlgF1 = true;
			MoveNotFlg = !MoveNotFlg;
		}
	}
	else
	{
		LookFlgF1 = false;
	}

	// ImGuiを非表示に
	if (GetAsyncKeyState(VK_F2))
	{
		if (!LookFlgF2) {
			LookFlgF2 = true;
			Flg_ImguiDraw = !Flg_ImguiDraw;
		}
	}
	else
	{
		LookFlgF2 = false;
	}

	//// 中央のオブジェクトが動かないように
	//if(MoveNotFlg)
	//	Flg_ObjMove = false;

	// 画面中央のにあるオブジェクトを判定
	if (frame % 3 <= 0)
	{
		// 画面中央のオブジェクトを判定
		CenterRay();
	}

	// フレームを加算
	frame++;

	if (frame >= 9) {
		frame = 0;
	}

	if (m_CenterRayResult.m_hit == false) { return; }

	//auto MoveObject = m_wpCenterObj.lock();			// オブジェクト操作用
	auto CenterObject = m_wpCenterObj.lock();			// オブジェクト操作用

	if (CenterObject != nullptr) {
		if (CenterObject->GetTag() & TAG_MoveObject)
			Flg_ObjMoveable = true;
		else
			Flg_ObjMoveable = false;
	}

	// 画面中央にステージ以外のオブジェクトがあれば2番目のレイデータを検索
	if (m_CenterRayResult.m_hit) {
		if (m_wpCenterObj.lock()->GetName_str() != "StageMap") {
		
			CenterRay2nd();
		}
		else
		{
			m_2ndRayResult = RayResult_Initialization;
		}
	}

	/*
	// 掴めるオブジェクトが中央にあるなら
	if(Flg_ObjMove)
	{
		// 計算に使う行列の取得
		RayInfo rayInfo;
		std::shared_ptr<CameraComponent> cameraConp = nullptr;

		// エディターカメラだったら
		if (m_editorCameraEnable)
		{
		}

		// カメラコンポーネントだったら
		else
		{
			cameraConp = m_wpTargetCamera.lock();
		}

		// カメラコンポーネントが有効だったら ------------------------------------------------------>
		if (cameraConp)
		{
			// カメラコンポーネントからレイ情報を作成			
			// 画面中央にレイを作成
			cameraConp->CreateRayInfoFormPlanPos(rayInfo,
				Math::Vector2(APP.m_window.GetWindowWidth() / 2.0f, APP.m_window.GetWindowHeight() / 2.0f));

			// レイの結果
			RayResult finalResult;
			std::shared_ptr<GameObject> targetObj = nullptr;	// InGuiが選択するオブジェクト

			// 全てのオブジェクトと判定
			for (auto& rObj : m_spObjects)
			{
				// プレイヤー以外
				if ((rObj->GetTag() & TAG_Player) == TAG_Player) { continue; }

				RayResult tmpResult;

				if (rObj->HitCheckByRay(rayInfo, tmpResult))
				{
					// より近いオブジェクトがあれば更新
					if (tmpResult.m_distance < finalResult.m_distance)
					{
						finalResult = tmpResult;

						targetObj = rObj;
					}
				}
			}

			// 何かのオブジェクトに当たったのあれば、選択オブジェクトに指定
			if (targetObj)
			{
				m_wpImguiSelectObj = targetObj;
			}

			auto MoveObject = m_wpImguiSelectObj.lock();

			m_CenterRayResult = finalResult;

			// 中央にあるのがStageの時は解除
			if (MoveObject->GetTag() == TAG_StageObject)
				Flg_ObjMove = false;
		}

	}
	else
	{
		// レイ情報初期化 (クリック時ののみ有効)
		//m_CenterRayResult = RayResult_Initialization;
	}
	*/
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=>

	auto selectObject = m_wpImguiSelectObj.lock();	// Imgui用

	if (Flg_ImguiDraw) {
		if (ImGui::Begin("Scene"))
		{
			// ImGui::Text(u8"今日はいい天気だから\n飛行機の座標でも表示しようかな。\n");	// u8 … UTF-8 "" … const cher

			// カメラの視点切り替え
			ImGui::Checkbox("EditorCamera", &m_editorCameraEnable);

			// オブジェクトリストの描画
			if (ImGui::CollapsingHeader("Object List", ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (auto&& rObj : m_spObjects)
				{
					// 選択オブジェクトと一致するオブジェクトかどうか
					bool selected = (rObj == selectObject);

					ImGui::PushID(rObj.get());

					// Selectableで選択できるように、 第二引数はハイライトの有無
					if (ImGui::Selectable(rObj->GetName(), selected))
					{
						m_wpImguiSelectObj = rObj;
					}

					ImGui::PopID();
				}
			}
		}
		ImGui::End();	// ウィンドウバー終了

		// フレームワーク
		if (ImGui::Begin("Frame"))
		{
			//フレームレートを表示
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}
		ImGui::End();

		if (ImGui::Begin("Inspector"))
		{
			if (selectObject)
			{
				// オブジェクトリストで選択したゲームオブジェクトの情報を描画
				selectObject->ImGuiUpdate();
			}
		}
		ImGui::End();
	}

	//// 右クリックを離す
	//if (!Flg_ObjMove){
	//	MoveObject = nullptr;
	//	Flg_2ndDistance = false;
	//	//ObjMove_Flg = false;
	//}

	std::shared_ptr<GameObject>	CenterObj = GetCenterObject().lock();

	
	// FPS視点の中心レイ情報
	if (m_spCamera)
	{
		std::shared_ptr<CameraComponent>spCamera =  m_wpTargetCamera.lock();

		if (Flg_ImguiDraw) {

			if (ImGui::Begin("Line of sight"))
			{
				//ImGui::Checkbox(u8"オブジェクト選択中", &Flg_ObjMove);

				/*
				//					　検知可能な距離		　　格納場所		　最小値　最大値
				ImGui::SliderFloat("Detecable distance", &spCamera->m_laserMaxRange, 1.0, 4.0);

				// 感知距離以内
				if (spCamera->GetDistance() < spCamera->m_laserMaxRange) {

					// ヒットしたオブジェクトがあるか
					if (spCamera->GetHitObj() != nullptr) {
						ImGui::Text(u8"Name   : %s\n", spCamera->GetHitObj()->GetName());
						ImGui::Text(u8"Distance : %.2f\n", spCamera->GetDistance());
					}
					else
						ImGui::Text(u8"None\n");
				}
				else
				{
					ImGui::Text(u8"None\n");
				}
				*/


				// 中央に移動可能オブジェクトがあったら
				if (CenterObj->GetTag() | TAG_MoveObject)
				{
					// 画面中央にあるオブジェクト
					ImGui::Text(u8"手前にあるオブジェクト\n");
					ImGui::Text(u8"Name　　　: %s\n", CenterObj->GetName());				// オブジェクト名
					ImGui::Text(u8"Distance　: %.2f\n", m_CenterRayResult.m_distance);		// 当たった距離

					// 2つ目に当たったら
					if (m_2ndRayResult.m_hit) {

						//区切り線
						ImGui::Separator();

						ImGui::Text(u8"後ろにあるオブジェクトまでの距離\n");
						ImGui::Text(u8"Distance　: %.2f\n", m_2ndRayResult.m_distance);		// 当たった距離

						if (!Flg_2ndDistance) {
							Flg_2ndDistance = true;	// 右クリックを離すと解除

							m_2ndDistance_First = m_2ndRayResult.m_distance;
						}

					}

					//ImGui::Text(u8"m_2ndDistance_First　: %.2f\n", m_2ndDistance_First);		// 当たった距離
				}

			}
			ImGui::End();
		}
	}

	// ログウィンドウ
	if (Flg_ImguiDraw) {
		m_Editor_Log.ImguiUpdate("Log Window");
		ImGuiPrefabFactoryUpdate();
	}

	// シェーダー <-----------------------------------------------------
	if (Flg_ImguiDraw) {

		// 光の調整
		if (ImGui::Begin("LightSettings"))
		{
			// 光源の向き
			if (ImGui::DragFloat3("Direction", &m_lightDir.x, 0.01f))
			{
				SHADER.m_cb8_Light.Work().DL_Dir = m_lightDir;
				SHADER.m_cb8_Light.Work().DL_Dir.Normalize();
			}
			// 光源色
			if (ImGui::SliderFloat3("Color", &m_lightColor.x, 0.0f, 1.0f))
			{
				SHADER.m_cb8_Light.Work().DL_Color = m_lightColor * m_lightBrightess;
			}
			// 輝度
			if (ImGui::DragFloat("Brightess", &m_lightBrightess, 0.01f))
			{
				SHADER.m_cb8_Light.Work().DL_Color = m_lightColor * m_lightBrightess;
			}

			//区切り線
			ImGui::Separator();

			// カラーピッカー
			float color_picker[3] = { m_lightColor.x, m_lightColor.y , m_lightColor.z };
			if (ImGui::ColorPicker3(u8"カラーピッカー", color_picker))
			{
				m_lightColor.x = color_picker[0];
				m_lightColor.y = color_picker[1];
				m_lightColor.z = color_picker[2];

				SHADER.m_cb8_Light.Work().DL_Color = m_lightColor * m_lightBrightess;
			}
		}
		ImGui::End();

		// Graphics Debug (シャドウマップを描画)
		if (ImGui::Begin("Graphics Debug"))
		{
			ImGui::Image((ImTextureID)SHADER.m_genShadowMapShader.GetDirShadowMap()->GetSRView(), ImVec2(200, 200));
		}
		ImGui::End();

	}
	// ----------------------------------------------------------------->

	if (Flg_ImguiDraw) {
		// プロジェクション座標変換率 (視野による拡大率) 確認
		if (ImGui::Begin("Projection Map"))
		{
			// ターゲットカメラを取得
			std::shared_ptr<CameraComponent> spTargetCamera = m_wpTargetCamera.lock();

			// 拡大率
			float sX = 0.0f;
			float sY = 0.0f;
			float sZ = 0.0f;
			float TransZ = 0.0f;

			sY = 1.0f / tan(spTargetCamera->GetViewingAngle() / 2.0f);
			sX = sY / APP.m_window.GetWindowAspectRatio();
			sZ = 1.0f / (spTargetCamera->GetFar() - spTargetCamera->GetNear()) * spTargetCamera->GetFar();

			TransZ = -spTargetCamera->GetNear() / (spTargetCamera->GetFar() - spTargetCamera->GetNear()) * spTargetCamera->GetFar();

			ImGui::Text("ScaleX = %f", sX);
			ImGui::Text("ScaleY = %f", sY);
			ImGui::Text("ScaleZ = %f", sZ);
			ImGui::Text("TransZ = %f", TransZ);
		}
		ImGui::End();

		// マウスホイール確認用

		if (ImGui::Begin("Mouse Wheel Val")) {

			ImGui::Text("Mouse Wheel Val = %d", APP.m_window.GetMouseWheelVal());
		}
		ImGui::End();
	}
}

// 前島先生課題 <---------------------------------------------------
void Scene::ImGuiPrefabFactoryUpdate()
{
	if (Flg_ImguiDraw) {

		if (ImGui::Begin("PrefabFactory"))
		{
			//ImGui::InputText("JsonName", &JsonFileName);
			ImGui::InputText("", &JsonFileName);

			// 同じ行に
			ImGui::SameLine();
			if (ImGui::Button(u8"Jsonパス取得"))
			{
				// ファイル選択
				Window::OpenFileDialog(JsonFileName, "Jsonパス取得", "Jsonパス\0*.json");
			}

			// 作成ボタンを押したら
			if (ImGui::Button("Create"))
			{
				if (JsonFileName != "")
				{
					LoadJsonData(JsonFileName);
					//m_Editor_Log.AddLog(u8"%s 生成成功\n", JsonFileName);
				}
			}
		}
		ImGui::End();
	}
}
// ----------------------------------------------------------------->

//デバックライン描画			始点					　終点					　色
void Scene::AddDebugLine(const Math::Vector3& p1, const Math::Vector3& p2, const Math::Color& color)
{
	// ラインの開始頂点
	EffectShader::Vertex ver;
	ver.Color = color;
	ver.UV = { 0.0f,0.0f };
	ver.Pos = p1;
	m_debugLines.push_back(ver);

	// ラインの終点頂点
	ver.Pos = p2;
	m_debugLines.push_back(ver);
}

void Scene::AddDebugSphereLine(const Math::Vector3& pos, float radius, const Math::Color& color)
{
	EffectShader::Vertex ver;
	ver.Color = color;
	ver.UV = { 0.0f,0.0f };

	static constexpr int kDetail = 16;
	for (UINT i = 0; i < kDetail + 1; ++i)
	{
		// XZ平面
		ver.Pos = pos;
		ver.Pos.x += cos((float)i * (360 / kDetail) * ToRadians) * radius;
		ver.Pos.z += sin((float)i * (360 / kDetail) * ToRadians) * radius;
		m_debugLines.push_back(ver);

		ver.Pos = pos;
		ver.Pos.x += cos((float)(i + 1) * (360 / kDetail) * ToRadians) * radius;
		ver.Pos.z += sin((float)(i + 1) * (360 / kDetail) * ToRadians) * radius;
		m_debugLines.push_back(ver);

		// XY平面
		ver.Pos = pos;
		ver.Pos.x += cos((float)i * (360 / kDetail) * ToRadians) * radius;
		ver.Pos.y += sin((float)i * (360 / kDetail) * ToRadians) * radius;
		m_debugLines.push_back(ver);

		ver.Pos = pos;
		ver.Pos.x += cos((float)(i + 1) * (360 / kDetail) * ToRadians) * radius;
		ver.Pos.y += sin((float)(i + 1) * (360 / kDetail) * ToRadians) * radius;
		m_debugLines.push_back(ver);

		// YZ平面
		ver.Pos = pos;
		ver.Pos.y += cos((float)i * (360 / kDetail) * ToRadians) * radius;
		ver.Pos.z += sin((float)i * (360 / kDetail) * ToRadians) * radius;
		m_debugLines.push_back(ver);

		ver.Pos = pos;
		ver.Pos.y += cos((float)(i + 1) * (360 / kDetail) * ToRadians) * radius;
		ver.Pos.z += sin((float)(i + 1) * (360 / kDetail) * ToRadians) * radius;
		m_debugLines.push_back(ver);
	}
}

// XYZ軸を描画
void Scene::AddDebugCoordinateAxisLine(const Math::Vector3& pos, float scale)
{
	// ラインの開始頂点
	EffectShader::Vertex ver;

	// 無くても可
	ver.Color = { 1.0f,1.0f,1.0f,1.0f };
	ver.UV = { 0.0f,0.0f };

	// X軸・赤色
	ver.Color = { 1.0f,0.0f,0.0f,1.0f };
	ver.Pos = pos;
	m_debugLines.push_back(ver);

	ver.Pos.x += 1.0f * scale;
	m_debugLines.push_back(ver);

	// Y軸・緑色
	ver.Color = { 0.0f,1.0f,0.0f,1.0f };
	ver.Pos = pos;
	m_debugLines.push_back(ver);

	ver.Pos.y += 1.0f * scale;
	m_debugLines.push_back(ver);

	// Z軸・青色
	ver.Color = { 0.0f,0.0f,1.0f,1.0f };
	ver.Pos = pos;
	m_debugLines.push_back(ver);

	ver.Pos.z += 1.0f * scale;
	m_debugLines.push_back(ver);
}

void Scene::RequestChangeScene(const std::string& fileName)
{
	// 次のシーンのJsonファイル名を覚える
	m_nextSceneFileName = fileName;

	// リクエストが有った事を覚える
	m_isRequestChangescene = true;
}

void Scene::ExecChangeScene()
{
	// シーンの遷移
	LoadObjectDatas(m_nextSceneFileName);
	//LoadScene(m_nextSceneFileName);

	// リクエスト状況のリセット
	m_isRequestChangescene = false;
}

// 座標変換の準備 (2D ⇔ 3D)	※[戻り値]　デフォルト：2D → 3D, Inverse()：3D → 2D
Matrix PreparingCoordinateTransformation(
	int screen_w,			// 画面の幅
	int screen_h,			// 画面の高さ
	const Matrix& rView,	// ビュー行列
	const Matrix& rPrj) {

	// 各行列を逆行列計算
	Matrix invView = rView;
	invView.Inverse();
	Matrix invPrj = rPrj;
	invPrj.Inverse();

	// ビューポートへ変換行列
	Matrix viewport;
	viewport._11 = screen_w / 2.0f;
	viewport._22 = -screen_h / 2.0f;
	viewport._41 = screen_w / 2.0f;
	viewport._42 = screen_h / 2.0f;

	// ビューポートの逆行列も計算
	Matrix invViewport = viewport;
	invViewport.Inverse();

	// 2D → 3D座標への変換行列
	Matrix convertMat = invViewport * invPrj * invView;

	return convertMat;
}

Vec3 Scene::ConvertScreenToWorld(
	int sx,					// スクリーン(ウィンドウ)上のX座標
	int sy,					// スクリーン(ウィンドウ)上のY座標
	float fZ,				// 射影空間でのZ値 (0-1)
	int screen_w,			// 画面の幅
	int screen_h,			// 画面の高さ
	const Matrix& rView,	// ビュー行列
	const Matrix& rPrj)		// 射影行列
{
	// 2D → 3D座標への変換行列
	Matrix convertMat = PreparingCoordinateTransformation(screen_w, screen_h, rView, rPrj);

	// 指定された座標を変換して返す
	Vec3 resultPos((float)sx, (float)sy, (float)fZ);

	Vec3 v = resultPos.TransformCoord(convertMat);
		
	return v;
}

Vec3 Scene::GetConvertScreenCenterToWorld(float fZ)
{
	std::shared_ptr<CameraComponent> spTargetCamera = m_wpTargetCamera.lock();

	return ConvertScreenToWorld(APP.m_window.GetWindowWidth() / 2.0f, APP.m_window.GetWindowHeight() / 2.0f, fZ, APP.m_window.GetWindowWidth(), APP.m_window.GetWindowHeight(), spTargetCamera->GetViewtMatrix(), spTargetCamera->GetProjtMatrix());
}

// 3D座標 から 2D座標へ変換
Vec3 Scene::ConvertWorldToScreen(Vec3 v, int screen_w, int screen_h, const Matrix& rView, const Matrix& rPrj)
{
	// 2D → 3D座標への変換行列
	Matrix convertMat = PreparingCoordinateTransformation(screen_w, screen_h, rView, rPrj);

	// 3D → 2D 座標への変換行列
	convertMat.Inverse();

	v.TransformCoord(convertMat);

	return v;
}

Vec3 Scene::GetConvertWorldToScreenCenter(Vec3 v) {

	std::shared_ptr<CameraComponent> spTargetCamera = m_wpTargetCamera.lock();

	return ConvertWorldToScreen(v, APP.m_window.GetWindowWidth(), APP.m_window.GetWindowHeight(), spTargetCamera->GetViewtMatrix(), spTargetCamera->GetProjtMatrix());
}


// 画面中央のレイ
void Scene::CenterRay()
{
	// 計算に使う行列の取得
	RayInfo rayInfo;
	std::shared_ptr<CameraComponent> cameraConp = nullptr;

	if (!m_editorCameraEnable)
		cameraConp = m_wpTargetCamera.lock();

	if (cameraConp) {
		// カメラコンポーネントからレイ情報を作成			
		// 画面中央にレイを作成
		cameraConp->CreateRayInfoFormPlanPos(rayInfo,
			Math::Vector2(APP.m_window.GetWindowWidth() / 2.0f, APP.m_window.GetWindowHeight() / 2.0f));

		// レイの結果
		RayResult finalResult;
		std::shared_ptr<GameObject> targetObj = nullptr;	// InGuiが選択するオブジェクト

		// 全てのオブジェクトと判定
		for (auto& rObj : m_spObjects)
		{
			// プレイヤー以外
			if ((rObj->GetTag() & TAG_Player) == TAG_Player) { continue; }

			//// オブジェクト生成中はそのオブジェクトを対象外に
			//if (ObjectCreateFlg) {
			//	if (rObj == m_spObjects.back()) { continue; }
			//}

			RayResult tmpResult;

			if (rObj->HitCheckByRay(rayInfo, tmpResult))
			{
				// より近いオブジェクトがあれば更新
				if (tmpResult.m_distance < finalResult.m_distance)
				{
					finalResult = tmpResult;

					targetObj = rObj;
				}
			}
		}

		// 何かのオブジェクトに当たったのあれば、選択オブジェクトに指定
		if (targetObj)
		{
			m_wpCenterObj = targetObj;
		}

		m_CenterRayResult = finalResult;
	}
}

void Scene::CenterRay2nd()
{
	// レイ情報初期化用
	RayResult RayResult_Initialization;

	// レイ情報初期化
	m_2ndRayResult = RayResult_Initialization;

	// 計算に使う行列の取得
	RayInfo rayInfo;
	std::shared_ptr<CameraComponent> cameraConp = nullptr;

	cameraConp = m_wpTargetCamera.lock();

	// カメラコンポーネントからレイ情報を作成			
	// 画面中央にレイを作成
	cameraConp->CreateRayInfoFormPlanPos(rayInfo,
		Math::Vector2(APP.m_window.GetWindowWidth() / 2.0f, APP.m_window.GetWindowHeight() / 2.0f));

	// レイの結果
	RayResult finalResult;
	std::shared_ptr<GameObject> centerObj = GetCenterObject().lock();

	// 全てのオブジェクトと判定
	for (auto& rObj : m_spObjects)
	{
		// プレイヤー、移動中のオブジェクト以外
		if ((rObj->GetTag() & TAG_Player) == TAG_Player) { continue; }
		if (rObj == centerObj) { continue; }

		RayResult tmpResult;

		// レイ判定
		if (rObj->HitCheckByRay(rayInfo, tmpResult))
		{
			// より近いオブジェクトがあれば更新
			if (tmpResult.m_distance < finalResult.m_distance)
			{
				finalResult = tmpResult;

				//targetObj = rObj;
			}
		}
	}

	// 2つ目に当たったオブジェクトの情報
	m_2ndRayResult = finalResult;
}

void Scene::Stage_GimmickUpdate() {

	//上昇制限
	static int	MoveY;
	static int	MoveFloorY;
	static int	MoveWall2Y;
	static int	MoveCeilingY;
	static bool	MoveFloorFlg;

	static int	frame;
	static bool	MoveFloor_Stop;

	//ノード別の移動 (テスト用)
	for (auto spObj : m_spObjects)
	{
		// ステージだったら
		if (spObj->GetTag() & TAG_StageObject) {
			if (spObj->GetName_str() == "StageMap") {

				// MoveObj_01のノードを上昇
				if (MoveY <= 500) {

					MoveY++;
					spObj->MoveNode(MoveNodeNameData::MoveWall_01, { 0.0f,0.01f,0.0f });
				}

				// 上下移動をする床
				if (MoveFloor_Stop)
				{
					if (frame >= 120)
						MoveFloor_Stop = false;

					frame++;
				}
				else
				{
					if (MoveFloorY <= 400 && !MoveFloorFlg) {

						MoveFloorY++;
						if (MoveFloorY >= 400)
						{
							MoveFloorFlg = true;
							MoveFloor_Stop = true;
							frame = 0;
						}

						spObj->MoveNode(MoveNodeNameData::MoveFloor_01, { 0.0f,-0.01f,0.0f });
					}
					else
					{
						MoveFloorY--;
						if (MoveFloorY <= 0)
						{
							MoveFloorFlg = false;
							MoveFloorY = 0;

							MoveFloor_Stop = true;
							frame = 0;
						}

						spObj->MoveNode(MoveNodeNameData::MoveFloor_01, { 0.0f,0.01f,0.0f });
					}
				}

				if (Scene::GetInstance().NowSwitchData & SwitchData::Switch_01)
				{
					if (MoveWall2Y <= 500) {
						MoveWall2Y++;
						spObj->MoveNode(MoveNodeNameData::Switch_01_Wall, { 0.0f,0.01f,0.0f });
					}
				}

				if ((Scene::GetInstance().NowSwitchData & SwitchData::Switch_02))
				{
					if (MoveCeilingY <= 90) {
						MoveCeilingY++;
						spObj->MoveNode(MoveNodeNameData::Switch_02_CannonCeiling, { 0.0f,-0.1f,0.0f });
					}
				}
			}

			//if (spObj->GetName_str() == "MaterialTestObject") {
			//	//spObj->SetNodeSubset("Cube", 0);
			//}
		}
	}
}