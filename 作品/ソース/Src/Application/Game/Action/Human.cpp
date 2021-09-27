#include "Human.h"

#include "../Scene.h"

#include "../../Component/CameraComponent.h"
#include "../../Component/InputComponent.h"
#include "../../Component/ModelComponent.h"
#include "GgimmickObject/Gimmick.h"

// 初期化 + メモリ確保
const float Human::s_allowToStepHeight = 0.1f;
const float Human::s_landingHeight = 0.1f;

void Human::Deserialize(const json11::Json& jsonObj)
{
	GameObject::Deserialize(jsonObj);	// 共通部分のDeserialize

	// カメラコンポーネントの設定 ( FPS視点 )
	if (m_spCameraComponent)
	{
		//m_spCameraComponent->OffsetMatrix().CreatTranslation(0.0f, 1.2f, 0.0f);
		//m_spCameraComponent->OffsetMatrix().RotateX(30.0f * ToRadians);

		m_spCameraComponent->OffsetMatrix().RotateX(10 * ToRadians);
	}

	// プレイヤーであればInputComponentを作成
	if ((GetTag() & TAG_Player) != 0)
	{
		Scene::GetInstance().SetTargetCamera(m_spCameraComponent);

		m_spInputComponent = std::make_shared<ActionPlayerInputComponent>(*this);
	}

	m_spActionState = std::make_shared<ActionWait>();

	//SetAnimation("Stand");

	FirstPos = m_pos;
	FirstPos.y += 2.0f;
}

void Human::SetAnimation(const char* pAnimName, bool isLoop)
{
	if (m_spModelComponent)
	{
		std::shared_ptr<AnimationData> animData = m_spModelComponent->GetAnimation(pAnimName);
		m_animator.SetAnimation(animData);
	}
}

void Human::Update()
{
	// Inputコンポーネントの更新
	if (m_spInputComponent)
	{
		m_spInputComponent->Update();
	}

	// 移動前の座標を覚える
	m_prevPos = m_pos;

	// カメラの更新 (マウス入力)
	UpdateCamera();

	// 入力による移動の更新
	if (m_spActionState)
	{
		m_spActionState->Update(*this);
	}

	//UpdateMove();

	// 重力をキャラクターのYの移動力に加える
	if (!this->IsGround()) {
		m_force.y -= m_gravity;
	}

	// 移動力をキャラクターの座標に足し込む
	m_pos.x += m_force.x;
	m_pos.y += m_force.y;
	m_pos.z += m_force.z;

	// 座標の更新を行った後に当たり判定
	UpdateCollision();

	// ワールド行列を合成する
	m_mWorld.CreateRotationX(m_rot.x);
	m_mWorld.RotateY(m_rot.y);
	m_mWorld.RotateY(m_rot.z);
	m_mWorld.Move(m_pos);

	m_mWorld.SetTranslation(m_pos);

	//Scene::GetInstance().m_PlayerMatrix = this->GetMatrix();

	// Cameraコンポーネントの更新
	if (m_spCameraComponent)
	{
		Matrix trans;
		trans.CreateTranslation(m_pos.x + FPS_LookatPoint_Correction.x, m_pos.y + FPS_LookatPoint_Correction.y, m_pos.z + FPS_LookatPoint_Correction.z);		// FPSカメラの注視点の位置 (CameraComponentの座標)

		m_spCameraComponent->SetCameraMatrix(trans);

		//m_spCameraComponent->UpdateRey();
	}

	// アニメーションの更新
	m_animator.AdvanceTime(m_spModelComponent->GetChangeableNoeds());

	//m_laser = (m_spInputComponent->GetButtoon(Input::Buttons::B) != InputComponent::FREE

	/*
	// 生成するオブジェクトを変更
	// Box
	if (m_spInputComponent->GetButtoon(Input::OperationButtons::Button_CreateBox)) {
		ObjectResourceData::CreateObjectJsonFileName = ObjectResourceData::CreateBoxJsonFileName;
	}
	// ばね
	if (m_spInputComponent->GetButtoon(Input::OperationButtons::Button_PowerSpring)) {
		ObjectResourceData::CreateObjectJsonFileName = ObjectResourceData::PowerSpringJsonFileName;
	}
	// 移動オブジェクト
	if (m_spInputComponent->GetButtoon(Input::OperationButtons::Button_MoveObj)) {
		ObjectResourceData::CreateObjectJsonFileName = ObjectResourceData::MoveBoxFileName;
	}
	*/
	/*
	if (ImGui::Begin("CreateObjectName")) {

		std::string CreateObj = "";
		if (ObjectResourceData::CreateObjectJsonFileName == ObjectResourceData::CreateBoxJsonFileName)
		{
			CreateObj = "CreateBox";
		}

		if (ObjectResourceData::CreateObjectJsonFileName == ObjectResourceData::PowerSpringJsonFileName)
		{
			CreateObj = "PowerSpring";
		}

		if (ObjectResourceData::CreateObjectJsonFileName == ObjectResourceData::MoveBoxFileName)
		{
			CreateObj = "MoveBox";
		}

		ImGui::Text(u8"CreateObj：%s", CreateObj.c_str());
	}
	ImGui::End();
	*/

	/*
	// 点光を追加
	Vec3 pos = m_mWorld.GetTranslation();
	pos.y += 1;
	SHADER.AddPointLight(pos, 5, { 10.0f,2.5f,10.0f });
	*/
}

void Human::Draw()
{
	GameObject::Draw();

	//if (m_laser)
	//{
		//// レーザーの終点を求める
		//Vec3 mStartPos = { m_prevPos.x, m_prevPos.y + 1.5f, m_prevPos.z };

		//Vec3 laserStart(mStartPos);
		//Vec3 laserEnd;
		//Vec3 laserDir(m_mWorld.GetAxisZ());

		//laserDir.Normalize();	// 拡大が入っていると1以上になるので正規化

		//laserDir *= m_laserRange;	//レーザーの射程方向ベクトルを伸ばす

		//laserEnd = laserStart + laserDir;	// レーザーの終点は発射ベクトル＋レーザーの長さ分

	//}


	if (m_spCameraComponent)
	{
		//Scene::GetInstance().AddDebugLine(m_spCameraComponent->GetlaserStart(), m_spCameraComponent->GetlaserEnd(), { 0.0f,1.0,1.0f,1.0f });

		//// レイの発射情報
		//RayInfo rayInfo;
		//rayInfo.m_pos = m_spCameraComponent->GetlaserStart();				// 移動する前の地点から
		//rayInfo.m_dir = m_spCameraComponent->GetCameratMatrix().GetAxisZ();	// 自分の向いている方向に
		//rayInfo.m_dir.Normalize();
		//rayInfo.m_maxRange = m_spCameraComponent->m_laserMaxRange;				// レーザー射程分判定

		//// レイの判定結果
		//RayResult finalRayResult;

		//// 全ゲームオブジェクトと判定
		//for (auto& obj : Scene::GetInstance().GetObjects())
		//{
		//	// 自分自身を無視
		//	if (obj.get() == this) { continue; }

		//	// 持てるオブジェクトが対象
		//	if (!(obj->GetTag() & (TAG_StageObject | TAG_AttackHit))) { continue; }

		//	RayResult rayResult;

		//	// 判定実行
		//	if (obj->HitCheckByRay(rayInfo, rayResult))
		//	{
		//		// 最も当たったところまでの距離が短いものを保持する
		//		if (rayResult.m_distance < finalRayResult.m_distance)
		//		{
		//			// 最短のレイ情報
		//			finalRayResult = rayResult;

		//			// 当たったオブジェクト
		//			m_spCameraComponent->SetHitObj(obj);
		//		}
		//	}
		//}

		//// 距離を格納
		//m_spCameraComponent->SetDistance(finalRayResult.m_distance);
	}
}

void Human::UpdateMove()
{
	// カメラの方向に移動方向が依存するので、カメラが無かったら帰る
	if (!m_spCameraComponent) { return; }

	// 入力情報の取得取得
	const Math::Vector2& inputMove = m_spInputComponent->GetAxis(Input::Axes::L);
	// カメラの右方向 * レバーの左右入力 = キャラクターの左右の移動方向
	Vec3 moveSide = m_spCameraComponent->GetCameratMatrix().GetAxisX() * inputMove.x;
	// カメラの前方向 * レバーの前後入力 = キャラクターの前後の移動方向
	Vec3 moveForward = m_spCameraComponent->GetCameratMatrix().GetAxisZ() * inputMove.y;

	// 上下方向への移動成分はカット (カメラは少し上下に傾いている為、カットしなければ地面に埋まったりしていく)
	moveForward.y = 0.0f;

	// 移動ベクトルの計算
	moveVec = { moveSide + moveForward };

	// 正規化
	moveVec.Normalize();

	// キャラクターの回転処理
	UpdateRotate(moveVec);

	// 移動速度に合わせる
	moveVec *= m_moveSpeed;

	// 場所に加算
	m_force.x = moveVec.x;
	m_force.z = moveVec.z;

	//// ジャンプ処理
	//if (m_spInputComponent->GetButtoon(Input::A) && m_isGround) {
	//	m_force.y = m_jumpPow;
	//}

	//// ばね処理
	//if (this->NowAction_GimmickType == GIMMICK_TAG::TAG_PowerSpring && m_isGround) {
	//	if (m_spInputComponent->GetButtoon(Input::A) && m_isGround) {

	//		//m_spInputComponent->ReleaceButton(Input::A);
	//		//{
	//		m_force.y = m_jumpPow * m_PowerSpringJump;
	//		//}
	//	}
	//	else
	//	{
	//		m_force.y = m_jumpPow;
	//	}
	//}
}

// カメラ視点の更新
void Human::UpdateCamera()
{
	//// 早期リターン
	if (!m_spCameraComponent) { return; }

	// マウス感度補正
	float dpi = 0.1f;

	const Math::Vector2& inputCamera = m_spInputComponent->GetAxis(Input::Axes::R);
	//if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
	//{
		float radX = inputCamera.x * ToRadians * dpi;
		float radY = inputCamera.y * ToRadians * dpi;

		m_spCameraComponent->OffsetMatrix().RotateY(radX);
		m_spCameraComponent->OffsetMatrix().RoteteAxis(m_spCameraComponent->OffsetMatrix().GetAxisX(), radY);

		ObjRot.RotateY(radX);
		ObjRot.Inverse();

		ObjRot.RoteteAxis(m_spCameraComponent->OffsetMatrix().GetAxisX(), radY);
	//}

	// エディターとターゲットカメラの切り替え
	if (m_spInputComponent->GetButtoon(Input::R3)) {
		m_editorCameraEnable = !m_editorCameraEnable;
	}

	// 1ループのみ作動させる為
	Obj_MoveEndFlg = false;

	// FPS視点時のみ
	if (!Scene::GetInstance().GetEditorCameraEnable()) {

		// 長押し防止用
		static bool LPPflg;

		// オブジェクトが移動可能なら
		if (Scene::GetInstance().Flg_ObjMoveable) {

			// 移動ボタンを押している
			if (m_spInputComponent->GetButtoon(Input::OperationButtons::Button_ObjMove))
			{
				if (LPPflg == false) {
					Obj_MoveFlg = !Obj_MoveFlg;

					// 移動終了
					if (!Obj_MoveEndFlg)
					{
						Obj_MoveEndFlg = true;
					}
				}

				LPPflg = true;
			}
			else
			{
				LPPflg = false;
			}

			//Obj_MoveFlg

			// オブジェクト移動
			MoveObject();
		}
		else
		{
			//if(m_spMoveObj != nullptr)
			//	MoveObject();

			// 過度な視点移動をした時、オブジェクトが中央からずれた時に自機が落ちないようにオブジェクトを手放す
			LPPflg			= false;
			Obj_MoveFlg		= false;
			Obj_MoveEndFlg	= false;
		}


		//if (ImGui::Begin("LButton"))
		//{			
		//	ImGui::Checkbox("LBooton", &Obj_MoveFlg);
		//	ImGui::Checkbox("Flg", &flg);
		//}
		//ImGui::End();

		//// 操作中に右クリックをすると解除
		//if (m_spInputComponent->GetButtoon(Input::OperationButtons::Button_ObjMove))
		//{
		//	if(!flg && Obj_MoveFlg)
		//		Obj_MoveFlg = false;
		//}
		//else
		//{
		//	flg = false;
		//}

		//MoveObject();
	}
	// オブジェクトの生成が可能なら
	//if (Obj_GeneratableFlg)
	//	CreateObject_ScreenCenter();

	//if (!m_spCameraComponent) { return; }

	//const Math::Vector2& inputCamera = m_spInputComponent->GetAxis(Input::Axes::R);

	//m_spCameraComponent->OffsetMatrix().RotateY(inputCamera.x * m_camRotSpeed * ToRadians);
}

// rMoveDir：移動方向
void Human::UpdateRotate(const Vec3& rMoveDir)
{
	// 移動していなければ帰る
	if (rMoveDir.LengthSquared() == 0.0f) { return; }

	// 今のキャラクターの方向ベクトル
	Vec3 nowDir = m_mWorld.GetAxisZ();
	nowDir.Normalize();

	// キャラクターの今向いている方向の角度を求める (ラジアン角)　※角度の大きさを取得
	float nowRadian = atan2(nowDir.x, nowDir.z);

	// 移動方向へのベクトルの角度を求める (ラジアン角)
	float targetRadian = atan2(rMoveDir.x, rMoveDir.z);

	float rotateRadian = targetRadian - nowRadian;

	// atan2の結果 = -π ～ π (-180 ～ 180度)

	// 180度の角度で数値の切れ目がある
	if (rotateRadian > M_PI)
	{
		rotateRadian -= 2 * float(M_PI);
	}
	else if (rotateRadian < -M_PI)
	{
		rotateRadian += 2 * float(M_PI);
	}

	//一回の回転角度を m_rotateAngle度以内に収める(クランプ)		※ clamp … 指定された2つの数値内に必ず収める処理　　例）Rot = 180; clamp(Rot, -45, 90); … Rotを-45から90以内の数値に修正 (Rot = 90になる)
	rotateRadian = std::clamp(rotateRadian, -m_rotateAngle * ToRadians, m_rotateAngle * ToRadians);

	rotateRadian = -rotateRadian;

	m_rot.y += rotateRadian;
}

// 当たり判定を更新
void Human::UpdateCollision()
{
	float distanceFromGround = FLT_MAX;

	// 下方向への判定を行い、着地した
	if (CheckGround(distanceFromGround))
	{
		// 地面の上にy座標を移動
		m_pos.y += s_allowToStepHeight - distanceFromGround;

		// 地面があるので、y方向(上下)への移動力は0に
		m_force.y = 0.0f;
	}

	// ぶつかり、差し戻し判定
	CheckBump();
}

// 地面との判定
bool Human::CheckGround(float& rDstDistance)
{
	// レイ判定情報
	RayInfo rayInfo;
	rayInfo.m_pos = m_pos; // キャラクターの位置を発射地点に　※キャラクターの原点 … 足

	// キャラの足元からレイを発射すると地面と当たらないので少し持ち上げる(乗り越えられる段差の高さ分だけ)
	rayInfo.m_pos.y += s_allowToStepHeight;

	// 落下中かもしれないので、１フレーム前の座標分も持ち上げる
	rayInfo.m_pos.y += m_prevPos.y - m_pos.y;

	// 地面方向へのレイ
	rayInfo.m_dir = { 0.0f, -0.7f, 0.0f };

	// デバッグ用
	//Scene::GetInstance().AddDebugLine(rayInfo.m_pos, rayInfo.m_pos + rayInfo.m_dir, { 0,1,1,1 });

	// レイの結果格納用
	rayInfo.m_maxRange = FLT_MAX;
	RayResult finalRayResult;		// 一番小さい値が入る

	// 当たったキャラクター
	std::shared_ptr<GameObject> hitObj = nullptr;

	// 全員とレイ判定
	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		// 自分自身は無視
		if (obj.get() == this) { continue; }

		// ステージと当たり判定（背景オブジェクト以外に乗る時は変更の可能性あり）
		//if (!(obj->GetTag() & (TAG_StageObject))) { continue; }

		//// スイッチ初期化
		//if (obj->GetTag() & TAG_Gimmick) {
		//	obj->SetGimmickBelowFlg(false);
		//	obj->SetGimmick1FrameFlg(false);
		//}

		RayResult rayResult;

		if (obj->HitCheckByRay(rayInfo, rayResult))
		{
			// 最も当たったところまでの距離が短いものを保持する
			if (rayResult.m_distance < finalRayResult.m_distance)
			{
				finalRayResult = rayResult;

				hitObj = obj;
			}
		}
	}

	if (hitObj != nullptr)
		if (hitObj->GetGimmickTag() != GIMMICK_TAG::TAG_PowerSpring)
			this->NowAction_GimmickType = GIMMICK_TAG::TAG_None;

	//if (m_spSwitch != nullptr) {
	//	if (hitObj->GetTag() & TAG_StageObject)
	//		m_spSwitch->SetSwitchFlg(true);
	//	else
	//		m_spSwitch->SetSwitchFlg(false);
	//}

	// 補正分の長さを結果に反映
	float distanceFromGround = FLT_MAX;

	// 足元にステージオブジェクトがあった
	if (finalRayResult.m_hit)
	{
		// 地面との距離を算出
		distanceFromGround = finalRayResult.m_distance - (m_prevPos.y - m_pos.y);
	}

	// 上方向に力がかかっていた場合
	if (m_force.y > 0.0f)
	{
		// 着地禁止
		m_isGround = false;
	}
	else
	{
		// 地面からの距離が (歩いて乗り越えられる高さ + 地面から足が離れていても着地する高さ) 未満であれば着地とみなす
		m_isGround = (distanceFromGround < (s_allowToStepHeight + s_landingHeight));
	}

	// 地面との距離を格納
	rDstDistance = distanceFromGround;

	// 動くものの上に着地した判定
	if (hitObj && m_isGround)
	{
		// 相手の一回動いた分を取得
		auto mOneMove = hitObj->GetOneMove();
		auto vOneMove = mOneMove.GetTranslation();

		// 相手の動いた分を自分の移動に
		m_pos += vOneMove;
	}

	//// 足元にあるオブジェクトがギミックだったら
	//if (m_isGround) {
	//	if (hitObj != nullptr)
	//	{
	//		static bool LookFlg;
	//		if (hitObj->GetTag() & TAG_Gimmick)
	//		{
	//			hitObj->SetGimmickBelowFlg(true);
	//			Hit_TAG = hitObj->GetGimmickTag();
	//		}
	//		else {
	//			hitObj->SetGimmickBelowFlg(false);
	//			Hit_TAG = GIMMICK_TAG::TAG_None;
	//			hitObj->SetGimmick1FrameFlg(false);
	//			LookFlg = false;
	//		}
	//	}
	//}

	this->m_hitUnderObj = hitObj;

	// 着地したかどうかを返す
	return m_isGround;
}

// ぶつかり判定 (横の当たり判定)
void Human::CheckBump()
{
	// 球情報の作成
	SphereInfo info;

	info.m_pos = m_pos;		// 中心点 = キャラクターの位置
	info.m_pos.y += 0.5f;	// キャラクターのぶつかり判定をするので、ちょっと上に持ち上げる
	info.m_radius = 0.4f;	// キャラの大きさに合わせて半径のサイズもいい感じに設定する

	// 確認のため判定場所をデバッグ表示
	//Scene::GetInstance().AddDebugSphereLine(info.m_pos, info.m_radius);

	// 当たったキャラクター
	std::shared_ptr<GameObject> hitObj = nullptr;

	SpherResult finalSpherResult;

	// 全てのオブジェクトと球面判定をする
	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		// 自分自身は無視
		if (obj.get() == this) { continue; }

		SpherResult spherResult;

		// 球による当たり判定
		if (obj->HitCheckBySpherVsMesh(info, spherResult))
		{
			// 初めに当たった物を基準とする (初回ヒット)
			if (finalSpherResult.m_push.LengthSquared() == 0)
				finalSpherResult = spherResult;


			if (spherResult.m_push.LengthSquared() < finalSpherResult.m_push.LengthSquared()) {
				finalSpherResult = spherResult;
				hitObj = obj;
			}
		}
	}

	if (finalSpherResult.m_hit)
		m_pos += finalSpherResult.m_push;
}

// 中央にあるオブジェクトを動かせる
void Human::MoveObject()
{
	//std::shared_ptr<GameObject> spHitObj;

	//// 画面中央にオブジェクトが無ければ
	//if (!Scene::GetInstance().GetCenterRayResult().m_hit) {

	//	if (m_spMoveObj != nullptr) {
	//		spHitObj = m_spMoveObj;
	//	}
	//	else
	//	{
	//		return;
	//	}
	//}
	//else
	//{
	//	// FPS時の正面にあるオブジェクト
	//	spHitObj = Scene::GetInstance().GetCenterObject().lock();
	//}

	// FPS時の正面にあるオブジェクト
	auto spHitObj = Scene::GetInstance().GetCenterObject().lock();

	// 正面にオブジェクトがなければ
	//if (HitObj == nullptr) { return; }

	// オブジェクトを動かせられない
	//if (!Scene::GetInstance().Flg_ObjMoveable) { return; };

	// 座標作成
	Vec3 ObjMatrix = spHitObj->GetMatrix().GetTranslation();				// 移動するオブジェクトの元の座標
	Vec3 ObjMat_2nd = Scene::GetInstance().Get2ndRayResult().rHitBeforePos;	// 2つ目に当たったオブジェクトの座標
	Vec3 ObjMatrix2 = ObjMatrix - ObjMat_2nd;								// 

	// 拡大率
	float scale = 0.0f;
	Vec3 Scale{1.0f,1.0f,1.0f};
	Vec3 OriginalScale{ spHitObj->GetMatrix().GetScale()};

	spHitObj->GetMatrix().GetScale().Normalize();

	// オブジェクト移動時の距離
	static float fZ;
	static float PrevfZ;	// 1フレーム前

	// ヒットしたオブジェクトのスクリーン上での座標
	Vec3 HitScreenPos{};

	// 現在の奥行を調べる
	HitScreenPos = Scene::GetInstance().GetConvertWorldToScreenCenter(spHitObj->GetMatrix().GetTranslation());

	// オブジェクトの奥行を格納
	fZ = HitScreenPos.z;

	// 移動オブジェクトの後ろにあるオブジェクトとの距離 (奥行の上限)
	static float fZ2;
	fZ2 = Scene::GetInstance().GetConvertWorldToScreenCenter(Scene::GetInstance().Get2ndRayResult().rHitPos).z;


	if (Obj_MoveFlg)
	{
		// マウスホイールの下回転
		if (APP.m_window.GetMouseWheelVal() < 0) {

			fZ -= 0.0001f;	// 引き寄せる

			scale = -0.01f;
			//scale = tanf(fZ);
			//scale = (Scene::GetInstance().Get2ndRayResult().m_distance - 5.0f) / 5.0f/ 10.0f;
		}
		// 上回転
		else if (APP.m_window.GetMouseWheelVal() > 0) {

			// 後ろにあるオブジェクトより手前なら (奥行の上限)
			if (fZ < fZ2) {

				fZ += 0.0001f;	// 引き離す

				// 拡大
				scale = 0.01f;
				//scale = (Scene::GetInstance().Get2ndRayResult().m_distance - 5.0f) / 7.5f / 10.0f;
			}
		}
		// 通常時は同じ距離を保つ
		else{
			fZ = PrevfZ;
		}


		//// 近い方を優先 (デフォルト値 or 現在の値)
		//if (fZ > HitScreenPos.z)
		//{
		//	fZ = HitScreenPos.z;
		//}

		if (fZ > fZ2)
		{
			fZ = fZ2;
		}

		//if (PrevfZ > fZ) {
		//	scale = -0.01f;
		//}
		//else if (PrevfZ < fZ) {
		//	scale = 0.01f;
		//}


		// カメラ中央の少し奥側 (デフォ 0.998f … 画面上 = 約4.0f離れる)
		Vec3 v = Scene::GetInstance().GetConvertScreenCenterToWorld(fZ);

		ObjMovePos.CreateTranslation(v);	// 空中

		//Scale = { 1.0f + scale, 1.0f + scale, 1.0f + scale };	// オブジェクトの拡大率
		Scale = { OriginalScale.x + scale, OriginalScale.y + scale, OriginalScale.z + scale };		// オブジェクトの座標


		ObjScale.CreateScaling(Scale);
		
		ObjMat = ObjScale * ObjMovePos;

		// 移動中の埋まり防止
		//spHitObj->CheckBump(ObjMat);


		// 座標更新
		spHitObj->SetMatrix(ObjMat);
	}
	else
	{
		// 移動後1度のみ
		if (Obj_MoveEndFlg)
		{
			ObjScale.CreateScaling(spHitObj->GetMatrix().GetScale());

			//ObjMovePos.CreateTranslation(ObjMat_2nd);	// 2つ目のオブジェクトの場所
			ObjMovePos.CreateTranslation(spHitObj->GetMatrix().GetTranslation());	// 2つ目のオブジェクトの場所

			ObjMat = ObjScale * ObjMovePos;

			// 移動中の埋まり防止
			//spHitObj->CheckBump(ObjMat);

			// 座標更新
			spHitObj->SetMatrix(ObjMat);
		}
	}

	// 前の座標として覚える
	PrevfZ = Scene::GetInstance().GetConvertWorldToScreenCenter(spHitObj->GetMatrix().GetTranslation()).z;

	if (Scene::GetInstance().GetFlg_ImguiDraw()) {

		if (ImGui::Begin("HitObj Data")) {
			//ImGui::Text("fZ   = %f", fZ);

			ImGui::Text("fZ2   = %f", fZ2);
			ImGui::Text("ScreenHitPos.z = %f", HitScreenPos.z);

			Vec3 scale = spHitObj->GetMatrix().GetScale();
			ImGui::Text("Scale : %0.2f, %0.2f, %0.2f", scale.x, scale.y, scale.z);
		}
		ImGui::End();
	}
}

// 画面中央にオブジェクトを生成
void Human::CreateObject_ScreenCenter()
{
	/*
	if (Scene::GetInstance().GetFlg_ImguiDraw()) { return; }

	auto HitObj = Scene::GetInstance().GetSelectObject().lock();

	// Boxを生成
	// 作成ボタンを押したら
	if (m_spInputComponent->GetButtoon(Input::OperationButtons::Button_CreateObj))
	{
		// 生成する場所
		Matrix CreateBoxMat = Matrix::Matrix();
		Vec3 ObjMat_2nd = Scene::GetInstance().Get2ndRayResult().rHitBeforePos;		// 2つ目に当たったオブジェクトの座標

		if (!Scene::GetInstance().ObjectCreateFlg) {

			Scene::GetInstance().ObjectCreateFlg = true;

			// jsonファイル名を元にオブジェクトを生成
			Scene::GetInstance().LoadJsonData(ObjectResourceData::CreateObjectJsonFileName);

			// Imguiのオブジェクトに指定
			//Scene::GetInstance().SetMoveObject(Scene::GetInstance().GetObjects().back());

			// 中心から飛ばしたレイで始めに当たった場所(1番手前)を格納
			Vec3 BoxCreatePos = Scene::GetInstance().GetCenterRayResult().rHitBeforePos;
			CreateBoxMat.CreateTranslation(BoxCreatePos);

			// 座標更新
			//CreateBoxMat.Move({ 0.01f,0.01f,0.01f });
			Scene::GetInstance().GetObjects().back()->SetMatrix(CreateBoxMat);
			Scene::GetInstance().GetObjects().back()->CheckBump();
		}
		else
		{
			// 他のオブジェクトがある場所に生成したか？
			if (Scene::GetInstance().Get2ndRayResult().m_hit)
			{
				// 2つ目のオブジェクトの場所に移動
				CreateBoxMat.CreateTranslation(ObjMat_2nd);
				HitObj->SetMatrix(ObjMat);
			}
			else
			{
				// 中心から飛ばしたレイで始めに当たった場所(1番手前)を格納
				Vec3 BoxCreatePos = Scene::GetInstance().GetCenterRayResult().rHitBeforePos;
				CreateBoxMat.CreateTranslation(BoxCreatePos);
			}

			// 座標更新
			//CreateBoxMat.Move({ 0.0f,0.01f,0.01f });
			Scene::GetInstance().GetObjects().back()->SetMatrix(CreateBoxMat);
			Scene::GetInstance().GetObjects().back()->CheckBump();
		}
	}
	else
	{
		Scene::GetInstance().ObjectCreateFlg = false;
	}
*/
}

void Human::ChangeStand()
{
	// 待機アクションへ変更
	m_spActionState = std::make_shared<ActionWait>();

	// 待機モーションへ変更
	//SetAnimation("Stand");
}

void Human::ChangeMove()
{
	// 移動アクションへ遷移
	m_spActionState = std::make_shared<ActionWalk>();

	// 歩きモーションへ変更
	//SetAnimation("Walk");
}

void Human::ChangeJump()
{
	m_force.y = m_jumpPow;

	// ジャンプアクションへ遷移
	m_spActionState = std::make_shared<ActionJamp>();
}

bool Human::IsChangeMove()
{
	if (!m_spInputComponent) { return false; }

	const Math::Vector2& inputMove = m_spInputComponent->GetAxis(Input::L);

	// 移動していたら
	if (inputMove.LengthSquared() != 0.0f)
	{
		return true;
	}

	return false;
}

bool Human::IsChangeJump()
{
	if (!m_spInputComponent) { return false; }

	if (CanJump())
	{
		if (m_spInputComponent->GetButtoon(Input::OperationButtons::Button_Jump))
		{
			return true;
		}
	}

	return false;
}

void Human::ActionWait::Update(Human& rOwner)
{
	rOwner.UpdateMove();

	// 移動する？
	if (rOwner.IsChangeMove())
	{
		// 移動アクションへ
		rOwner.ChangeMove();

		return;
	}

	if (rOwner.IsChangeJump())
	{
		// ジャンプする？
		rOwner.ChangeJump();

		return;
	}
}

void Human::ActionWalk::Update(Human& rOwner)
{
	rOwner.UpdateMove();

	if (!rOwner.IsChangeMove())
	{
		// 待機アクションへ変更
		rOwner.ChangeStand();
	}

	if (rOwner.IsChangeJump())
	{
		// ジャンプする？
		rOwner.ChangeJump();

		return;
	}
}

void Human::ActionJamp::Update(Human& rOwner)
{
	rOwner.UpdateMove();

	if (rOwner.IsGround())
	{
		// 待機アクションへ遷移
		rOwner.m_spActionState = std::make_shared<ActionWait>();

		return;
	}
}