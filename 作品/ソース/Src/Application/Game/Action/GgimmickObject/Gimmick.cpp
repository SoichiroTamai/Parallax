#include "Gimmick.h"
#include "../../Scene.h"

void Gimmick::Deserialize(const json11::Json& jsonObj)
{
	GameObject::Deserialize(jsonObj);
}

void Gimmick::Update() {
	GameObject::Update();

	// 移動制限用フラグ (１フレームのみ移動 )
	//static bool moveFlg = false;

	// 上にギミックがあれば稼働
	if (this->CheckObject_on_Top()) {

		switch (this->GetGimmickTag())
		{
			// スイッチ
			case GIMMICK_TAG::TAG_Switch:
				if (!this->m_gimmickMoveFlg) {
					this->m_gimmickMoveFlg = true;
					this->m_mWorld._42 -= 0.3f;

					// 対応しているビットを立てる
					if (this->GetName_str() == SwitchData::SwitchName_01)
						Scene::GetInstance().SetNowSwitchData(SwitchData::Switch_01);
						//SwitchData::NowSwitchData = SwitchData::Switch_01;
					else if (this->GetName_str() == SwitchData::SwitchName_02)
						Scene::GetInstance().SetNowSwitchData(SwitchData::Switch_02);
				}
				break;

			// 移動
			case GIMMICK_TAG::TAG_Move:

				if (CheckObject_Hit())
					this->Destroy();

				if(GimmickTopResult.HitObj->GetForce().y == 0.0f)
				this->m_mWorld._43 += 0.02f;
				break;

			//case GIMMICK_TAG::TAG_PowerSpring:
			//	//if(GimmickTopResult.HitObj->GetIsGround())
			//	//	GimmickTopResult.HitObj->MoveObj({0.0f,1.5f,0.0f});
			//	break;

			/* ばね 
				Huma::m_PowerSpringJump にて強さ設定
			*/

			//case GIMMICK_TAG::TAG_Cannon:
			//	CannonUpdate();
			//	break;
		}
	}
	else {

		// スイッチリセット
		if (this->GetGimmickTag() == GIMMICK_TAG::TAG_Switch && this->m_gimmickMoveFlg)
		{
			// 対応しているビットを下げる
			if (this->GetName_str() == SwitchData::SwitchName_01)
				Scene::GetInstance().SetNowSwitchData(-SwitchData::Switch_01);
			else if (this->GetName_str() == SwitchData::SwitchName_02)
				Scene::GetInstance().SetNowSwitchData(-SwitchData::Switch_02);


			this->m_gimmickMoveFlg = false;
			this->m_mWorld.SetTranslation(this->m_mFirstPos);
		}
	}

	if (this->GetGimmickTag() == GIMMICK_TAG::TAG_Cannon) {
		if(CheckCannon())
			CannonUpdate();
	}
}

bool Gimmick::CheckObject_on_Top()
{
	bool hitFlg = false;

	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		// 自分自身とステージは無視
		if (obj.get() == this) { continue; }
		if (obj->GetHitUnderObj() == nullptr) { continue; }

		//if (obj->GetName_str() == "GameObject") { continue; }
		//if (obj->GetName_str() == "StageMap") { continue; }
		//if (obj->GetGimmickTag() == GIMMICK_TAG::TAG_Switch ) { continue; }

		// オブジェクト生成中
		if (Scene::GetInstance().ObjectCreateFlg) {	continue; }


		// 重力がかかっていない(着地している)
		//if (obj->GetForce().y == 0.0f) {
		//if (obj->GetIsGround()) {
			// 下にあるオブジェクトが自身(ギミックがだったら)
			if (obj->GetHitUnderObj()->GetName() == this->GetName()) {

				//if (this->GetGimmickTag() == GIMMICK_TAG::TAG_Switch) {
				//	//if (this->m_mWorld._42 == this->m_mFirstPos.y)
				//		hitFlg = true;
				//}

				//switch (this->GetGimmickTag())
				//{
				//case GIMMICK_TAG::TAG_PowerSpring:
				//	break;

				//}

				if (this->GetGimmickTag() == GIMMICK_TAG::TAG_PowerSpring) {
					

					obj->SetGimmickType(GIMMICK_TAG::TAG_PowerSpring);

					//if(obj->GetForce().y == 0.0f)
					//	obj->MoveObj({ 0.0f,0.2f,0.0f });
					

					//if (obj->GetForce().y == 0.0f)
					//{
					//	float g = 0.5f;

					//	if (GetAsyncKeyState(VK_SPACE))
					//		g = 1.0f;

					//	obj->MoveObj({ 0.0f, g, 0.0f });

					//}

				}

				hitFlg = true;
				GimmickTopResult.HitObj = obj;

			}

		//}
	}
	
	GimmickTopResult.HitFlg = hitFlg;

	//if(hitFlg == false)
		//GimmickTopResult.HitObj->SetGimmickType(GIMMICK_TAG::TAG_None);

	return hitFlg;

	/*
	// レイ判定情報
	RayInfo rayInfo;
	rayInfo.m_pos = this->m_mWorld.GetTranslation(); // キャラクターの位置を発射地点に　※キャラクターの原点 … 足

	// 下からレイを発射すると他のオブジェクトと当たらないので少し持ち上げる
	rayInfo.m_pos.y += 0.01f;

	// 落下中かもしれないので、１フレーム前の座標分も持ち上げる
	rayInfo.m_pos.y += this->m_mPrev._42 - this->m_mWorld._42;

	// 地面方向へのレイ
	rayInfo.m_dir = { 0.0f, 0.5f, 0.0f };

	// デバッグ用
	Scene::GetInstance().AddDebugLine(rayInfo.m_pos, rayInfo.m_pos + rayInfo.m_dir, { 1,0,1,1 });

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
		if (obj->GetName_str() == "StageMap") { continue; }
	
		RayResult rayResult;

		if (obj->HitCheckByRay(rayInfo, rayResult)) {
			
			// 最も当たったところまでの距離が短いものを保持する
			if (rayResult.m_distance < finalRayResult.m_distance)
			{
				finalRayResult = rayResult;

				hitObj = obj;
			}
		}
	}


	//if (hitObj != nullptr)
	//{
	//	if (!hitObj->GetIsGround()) {
	//		finalRayResult.m_hit = false;
	//	}
	//}

	return finalRayResult.m_hit;
	*/
}

bool Gimmick::CheckObject_Hit()
{
	// 球情報の作成
	SphereInfo info;

	info.m_pos = this->m_mWorld.GetTranslation();	// 中心点 = キャラクターの位置
	info.m_radius = 0.6f;							// キャラの大きさに合わせて半径のサイズもいい感じに設定する

	// 確認のため判定場所をデバッグ表示
	//Scene::GetInstance().AddDebugSphereLine(info.m_pos, info.m_radius, DirectX::SimpleMath::Color{ 0.5f,0.5f,0.5f,1.0f });

	SpherResult finalSpherResult;

	// 全てのオブジェクトと球面判定をする
	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		// 自分自身は無視
		if (obj.get() == this) { continue; }
		if (obj == GimmickTopResult.HitObj) { continue; }

		SpherResult spherResult;

		// 球による当たり判定
		if (obj->HitCheckBySpherVsMesh(info, spherResult))
		{
			return true;
		}
	}

	return false;
}



bool Gimmick::CheckCannon()
{
	// 当たったキャラクター
	std::shared_ptr<GameObject> TargetObj = nullptr;
	
	// 一番近い距離
	float NearestLength = FLT_MAX;

	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		if (obj.get() == this) { continue; }
		if (obj->GetTag() == TAG_StageObject) { continue; }
		if (obj->GetGimmickTag() == GIMMICK_TAG::TAG_Shell) { continue; }
		
		Vec3 Length;
		Length = this->GetMatrix().GetTranslation() - obj->GetMatrix().GetTranslation();


		// 検知範囲内か (2乗の値が入る)
		if (Length.LengthSquared() <= 49.0f)
		{
			if (Length.LengthSquared() < NearestLength) {
				NearestLength = Length.LengthSquared();
				this->GimmickCannonResult.HitFlg = true;
				TargetObj = obj;
			}
		}
	}

	if (this->GimmickCannonResult.HitFlg) {
		this->GimmickCannonResult.HitObj = TargetObj;
	}

	return this->GimmickCannonResult.HitFlg;
}

void Gimmick::CannonUpdate()
{
	//std::shared_ptr<GameObject> sg;

	if (!this->GimmickCannonResult.HitFlg) { return; }
	//if (!this->GimmickCannonResult.HitObj == sg) { return; }

	Gimmick_Fps++;

	if (Gimmick_Fps % 180 == 0) {
		
		// jsonファイル名を元にオブジェクトを生成
		Scene::GetInstance().LoadJsonData(ObjectResourceData::ShellJsonFileName);
		
		// 基準となる初期座標を作成1
		Matrix FastMat = this->GetMatrix();
		FastMat._42 += 1.3f;
		Scene::GetInstance().GetObjects().back()->SetMatrix(FastMat);
		
		// 砲弾からタ―ゲットへのベクトル
		Vec3 FirstMoment = this->GimmickCannonResult.HitObj->GetMatrix().GetTranslation() - this->GetMatrix().GetTranslation();
		FirstMoment.Normalize();

		// 自身のZ軸方向(前方向)
		Vec3 vZ = this->m_mWorld.GetAxisZ();
		vZ.Normalize();
		
 		Math::Vector2 v1 = Math::Vector2(this->GimmickCannonResult.HitObj->GetMatrix().GetTranslation().x, this->GimmickCannonResult.HitObj->GetMatrix().GetTranslation().z);
		Math::Vector2 v2 = Math::Vector2(this->GetMatrix().GetTranslation().x, this->GetMatrix().GetTranslation().z);

		float rad = atan2(v2.y - v1.y, v2.x - v1.x);
		float dec = rad * 180.0f / M_PI;

		//if (dec < 0) { dec = dec + 90.0f; }

		// 今の前方向のベクトルをターゲット方向のベクトルに 1.0 度傾ける
		vZ.Complement(FirstMoment, dec);

		vZ.y = 0.0f;

		Scene::GetInstance().GetObjects().back()->SetWorldAxisZ(vZ);

		Vec3 move = Scene::GetInstance().GetObjects().back()->GetMatrix().GetAxisZ();		// 前方向に向く
		move.Normalize();	// 向きの長さを1に

		move *= 0.1f;

		Scene::GetInstance().GetObjects().back()->SetForce(move);
		//Scene::GetInstance().GetObjects().back()->MoveObj(FirstMoment);
	}
}