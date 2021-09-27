#include "GameObject.h"
#include "../Component/CameraComponent.h"
#include "../Component/InputComponent.h"
#include "../Component/ModelComponent.h"

// オブジェクト読み込み用
#include "Action/ActionGameProcess.h"

#include "Action//Human.h"
#include "Action/Lift.h"
#include "Action/Object.h"
#include "Action/Object.h"
#include "Action//GgimmickObject/Gimmick.h"

#include "Scene.h"

// 初期化 + メモリ確保
const float GameObject::s_allowToStepHeight = 0.1f;
const float GameObject::s_landingHeight = 0.1f;

// コンストラクタ
GameObject::GameObject() {}

// デストラクタ
GameObject::~GameObject() {}

// ゲッター・セッター <--------------------------------------------
// ノード情報を取得
ModelInfo::Node* GameObject::GetNodeData(std::string NodeName) {
	return this->GetModelComponent()->FindNode(NodeName);
}
// -----------------------------------------------------------------

// データの復元
void GameObject::Deserialize(const json11::Json& jsonObj)
{
	if (jsonObj.is_null()) { return; }

	// オブジェクト名 ------------------------
	if (jsonObj["Name"].is_null() == false)
	{
		m_name = jsonObj["Name"].string_value();
	}

	// オブジェクト識別用タグ ----------------
	if (jsonObj["Tag"].is_null() == false)
	{
		m_tag = jsonObj["Tag"].int_value();
	}
	
	// ギミック識別用タグ --------------------
	if (jsonObj["GimmickTag"].is_null() == false)
	{
		m_gimmick_tag = jsonObj["GimmickTag"].int_value();
	}
	
	// モデル --------------------------------
	if (m_spModelComponent)
	{
		m_spModelComponent->SetModedl(ResFac.GetModel(jsonObj["ModelFileName"].string_value()));
	}
	
	// 行列 =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	Matrix mTrans, mRotate, mScale;

	// 座標  --------------------------------
	const std::vector<json11::Json>& rPos = jsonObj["Pos"].array_items();
	if (rPos.size() == 3)
	{
		mTrans.CreateTranslation((float)rPos[0].number_value(), (float)rPos[1].number_value(), (float)rPos[2].number_value());
	}
	
	// 回転  --------------------------------
	const std::vector<json11::Json>& rRot = jsonObj["Rot"].array_items();
	if (rRot.size() == 3)
	{
		mRotate.CreateRotationX((float)rRot[0].number_value() * ToRadians);
		mRotate.RotateY((float)rRot[1].number_value() * ToRadians);
		mRotate.RotateZ((float)rRot[2].number_value() * ToRadians);
	}
	
	// 拡大 --------------------------------
	const std::vector<json11::Json>& rScale = jsonObj["Scale"].array_items();
	if (rScale.size() == 3)
	{
		mScale.CreateScaling((float)rScale[0].number_value(), (float)rScale[1].number_value(), (float)rScale[2].number_value());
	}

	m_mWorld = mScale * mRotate * mTrans;

	// 初期座標格納
	m_mFirstPos = m_mWorld.GetTranslation();
}

// 更新
void GameObject::Update()
{
	// 動く前の行列を覚える
	m_mPrev = m_mWorld;

	// override している為 無くても良い (自機はこのUpdateを通らない)
	if (this->GetTag() & (TAG_Player)) { return; }

	// ステージは移動しない
	if (this->GetName_str() == "StageMap") { 
	
		if (this->m_force.y < 0.0f) {
			this->m_force.y = 0.0f;
		}

		//if (this->m_mWorld.GetTranslation().isNE( this->m_mPrev.GetTranslation()) ) {
		//
		//	this->m_mWorld.SetTranslation(this->m_mPrev.GetTranslation());
		//}

		if (this->m_mWorld._41 != 0.0f) {
			this->m_mWorld._41 = 0.0f;
		}

		if (this->m_mWorld._42 < 0.0f) {
			this->m_mWorld._42 = 0.0f;
		}

		if (this->m_mWorld._43 != 0.0f) {
			this->m_mWorld._43 = 0.0f;
		}

		return;
	}
	
	//if (this->GetGimmickTag() & GIMMICK_TAG::TAG_Move) {
	//	return;
	//}

	if (!(this->GetTag() & TAG_StageObject)) {

		// Y軸は重力と同時に加算
		m_mWorld.Move({ m_force.x, 0.0f, m_force.z });

		//// オブジェクト生成中？
		//if (Scene::GetInstance().ObjectCreateFlg)
		//{
		//	if (!m_isGround) {

		//		//生成中のオブジェクトは対象外に
		//		if (this != Scene::GetInstance().GetObjects().back().get()) {

		//			if (!m_isGround) {

		//				// 着地していなかったら
		//				UpdateGravity();
		//				//UpdateCollision_G();
		//			}
		//		}

		//		UpdateCollision();					//UpdateGravity();
		//	}
		//}
		//else
		//{
			// 着地していなかったら
			if (!m_isGround) {

				UpdateGravity();
				
				UpdateCollision();
			}
		//}

		//UpdateCollision_G();
		//CheckBump_G();
	}
}

// 描画
void GameObject::Draw()
{
	if (m_spModelComponent == nullptr) { return; }
	m_spModelComponent->Draw();
}

void GameObject::DrawShadowMap()
{
	if (m_spModelComponent == nullptr)return;
	m_spModelComponent->DrawShadowMap();
}

// ImGui更新
void GameObject::ImGuiUpdate()
{
	ImGui::InputText("Name", &m_name);

	// Tag
	if (ImGui::TreeNodeEx("Tag", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::TreeNodeEx(u8"キャラクター関連", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::CheckboxFlags("Character", &m_tag, TAG_Character);
			ImGui::CheckboxFlags("Player", &m_tag, TAG_Player);
			ImGui::CheckboxFlags("StageObject", &m_tag, TAG_StageObject);
			ImGui::CheckboxFlags("MoveObject", &m_tag, TAG_MoveObject);
			ImGui::CheckboxFlags("Gimmick", &m_tag, TAG_Gimmick);

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx(u8"当たり判定関連", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::CheckboxFlags("AttackHit", &m_tag, TAG_AttackHit);

			ImGui::TreePop();
		}

		// テキストにコピー(保存)
		if (ImGui::Button(u8"JSONテキストコピー"))
		{
			ImGui::SetClipboardText(Format("\"Tag\":%d", m_tag).c_str());	// \はJsonの中の " と同じ	Format ≒ printf
		}

		ImGui::TreePop();
	}

	// Transform
	if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Vec3 pos = m_mWorld.GetTranslation();
		Vec3 rot = m_mWorld.GetAngles() * ToDegrees;
		Vec3 scale = m_mWorld.GetScale();

		bool isCange = false;

		isCange |= ImGui::DragFloat3("Position", &pos.x, 0.01f);
		isCange |= ImGui::DragFloat3("Rotation", &rot.x, 0.1f);
		isCange |= ImGui::DragFloat3("Scale", &scale.x, 0.1f);

		if (isCange)
		{
			// 拡大率の下限値
			if (scale.x <= 0.0f) { scale.x = 0.001f; }
			if (scale.y <= 0.0f) { scale.y = 0.001f; }
			if (scale.z <= 0.0f) { scale.z = 0.001f; }

			Matrix mS;
			mS.CreateScaling(scale.x, scale.y, scale.z);

			// 計算するときはRaddianに戻す
			rot *= ToRadians;

			Matrix mR;
			mR.RotateX(rot.x);
			mR.RotateY(rot.y);
			mR.RotateZ(rot.z);

			mR.SetScale(scale);

			m_mWorld = mR;

			m_mWorld.SetTranslation(pos);
		}

		// テキストにコピー (保存)
		if (ImGui::Button(u8"JSONテキストコピー"))
		{
			std::string s = Format("\"Pos\":[%.1f,%.1f,%.1f],\n", pos.x, pos.y, pos.z);
			s += Format("\"Rot\":[%.1f,%.1f,%.1f],\n", rot.x, rot.y, rot.z);
			s += Format("\"Scale\":[%.1f,%.1f,%.1f],\n", scale.x, scale.y, scale.z);
			ImGui::SetClipboardText(s.c_str());
		}

		ImGui::TreePop();
	}
}

// 球による当たり判定(距離判定)
bool GameObject::HitCheckBySphere(const SphereInfo& rInfo)
{
	// 当たっとする距離の計算(お互いの半径を足した値)
	float hitRange = rInfo.m_radius + m_colRadius;

	//自分の座標ベクトル
	Vec3 myPos = m_mWorld.GetTranslation();

	// 二点間のベクトルを計算
	Vec3 betweenVec = rInfo.m_pos - myPos;

	// 二点間の距離を計算
	float distance = betweenVec.Length();

	bool isHit = false;

	if (distance <= hitRange)
	{
		isHit = true;
	}

	return isHit;
}

// レイによる当たり判定
bool GameObject::HitCheckByRay(const RayInfo& rInfo, RayResult& rResult)
{
	// 判定する対象のない場合は当っていないとして帰る
	if (!m_spModelComponent) { return false; }

	// モデルコンポーネントはインスタンス化されているが、メッシュ情報を読み込んでなかった場合も変える

	// 全てのノード ( メッシュ ) 分当たり判定を行う
	for (auto& node : m_spModelComponent->GetNodes())
	{
		if (!node.m_spMesh) { continue; }

		//// StageMapの場合 Fence は対象外に
		//if (this->GetName_str() == "StageMap") {
		//	if (node.m_name == "Fence")
		//	{
		//		continue;
		//	}
		//}

		RayResult tmpResult;	// 結果

		// レイ判定（本体からのズレも加味して計算）
		RayToMesh(rInfo.m_pos, rInfo.m_dir, rInfo.m_maxRange, *(node.m_spMesh),
			node.m_localTransform * m_mWorld, tmpResult);

		// より近い判定を優先する
		if (tmpResult.m_distance < rResult.m_distance)
		{
			rResult = tmpResult;
		}
	}

	return rResult.m_hit;
}

// 球による当たり判定 (メッシュ)
bool GameObject::HitCheckBySpherVsMesh(const SphereInfo& rInfo, SpherResult& rResult)
{
	// モデルコンポーネントがない場合は判定しない
	if (!m_spModelComponent) { return false; }

	// 全てのノードのメッシュから押し返された位置を格納する座標ベクトル
	Vec3 pusheFromNodesPos = rInfo.m_pos;

	// 全てのノードと判定
	for (auto& node : m_spModelComponent->GetNodes())
	{
		// このノードがモデルを持っていなかった場合無視
		if (!node.m_spMesh) { continue; }

		// 点とノードの判定
		if (SphereToMesh(pusheFromNodesPos, rInfo.m_radius, *node.m_spMesh, node.m_localTransform * m_mWorld, pusheFromNodesPos))
		{
			rResult.m_hit = true;
		}
	}

	// 当たっていたら
	if (rResult.m_hit)
	{
		// 押し戻された球の位置と前の位置から、押し戻すベクトルを計算する
		rResult.m_push = pusheFromNodesPos - rInfo.m_pos;
	}

	return rResult.m_hit;
}

// 重力加算
void GameObject::UpdateGravity()
{
	// 重力をオブジェクトのYの移動力に加える
	m_force.y -= m_gravity;

	//if (this->GetGimmickTag() == GIMMICK_TAG::TAG_Shell)
	//	m_force.y = 0.0f;

	// 移動力をキャラクターの座標に足し込む
	this->m_mWorld._41 += m_force.x;
	this->m_mWorld._42 += m_force.y;
	this->m_mWorld._43 += m_force.z;
}

// コリジョン判定 (当たり判定)
void GameObject::UpdateCollision()
{
	float distanceFromGround = FLT_MAX;

	// 下方向への判定を行い、着地した
	if (CheckGround(distanceFromGround))
	{
		// 地面の上にy座標を移動
		//this->m_mWorld._42 += s_allowToStepHeight - distanceFromGround;
		this->m_mWorld._42 += distanceFromGround;

		// 地面があるので、y方向(上下)への移動力は0に
		m_force.y = 0.0f;
	}

	if (!Scene::GetInstance().ObjectCreateFlg)
	{
		// ぶつかり、差し戻し判定
		CheckBump();
	}
}

// 地面との判定
bool GameObject::CheckGround(float& rDstDistance)
{
	// レイ判定情報
	RayInfo rayInfo;
	rayInfo.m_pos = this->m_mWorld.GetTranslation(); // キャラクターの位置を発射地点に　※原点

	// キャラの足元からレイを発射すると地面と当たらないので少し持ち上げる(乗り越えられる段差の高さ分だけ)
	rayInfo.m_pos.y += 0.01f;

	// 落下中かもしれないので、１フレーム前の座標分も持ち上げる
	rayInfo.m_pos.y += m_mPrev.GetTranslation().y - this->m_mWorld.GetTranslation().y;

	// 地面方向へのレイ
	rayInfo.m_dir = { 0.0f, -1.0f, 0.0f };

	//Scene::GetInstance().AddDebugLine(rayInfo.m_pos, rayInfo.m_pos + rayInfo.m_dir,{0,1,1,1});

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

	// 補正分の長さを結果に反映
	float distanceFromGround = FLT_MAX;

	// 足元にステージオブジェクトがあった
	if (finalRayResult.m_hit)
	{
		// 地面との距離を算出
		distanceFromGround = finalRayResult.m_distance - (m_mPrev.GetTranslation().y - this->m_mWorld.GetTranslation().y);
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
		this->m_mWorld.GetTranslation() += vOneMove;
	}

	this->m_hitUnderObj = hitObj;

	// 着地したかどうかを返す
	return m_isGround;
}

// ぶつかり判定 (横の当たり判定)
void GameObject::CheckBump()
{
	if (this->GetName_str() == "StageMap") {
		return;
	}
	if (this->GetTag() == TAG_Player) {
		return;
	}

	// 球情報の作成
	SphereInfo info;

	// 中心点 = キャラクターの位置
	info.m_pos = this->m_mWorld.GetTranslation();	
	
	// 判定範囲　※サイズが変わるオブジェクトは拡大率に応じてサイズ変更
	if (this->GetTag() == TAG_MoveObject) {
		info.m_radius = 1.5f * (((this->GetMatrix().GetScale().x * this->GetMatrix().GetScale().y * this->GetMatrix().GetScale().z) / 3.0f) * 0.8f);				// キャラの大きさに合わせて半径のサイズもいい感じに設定する
		//info.m_radius = 1.5f * ((this->GetMatrix().GetScale().x * this->GetMatrix().GetScale().y * this->GetMatrix().GetScale().z) / 3.0f);						// キャラの大きさに合わせて半径のサイズもいい感じに設定する
		
		if (info.m_radius <= 0.1) {
			info.m_radius = 0.1f;
		}

	}
	else {
		info.m_radius = 0.7f;
	}

	// 確認のため判定場所をデバッグ表示
	//Scene::GetInstance().AddDebugSphereLine(info.m_pos, info.m_radius, DirectX::SimpleMath::Color{1,0,0,1});

	// 当たったキャラクター
	std::shared_ptr<GameObject> hitObj = nullptr;

	SpherResult finalSpherResult;

	// 全てのオブジェクトと球面判定をする
	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		// 自分自身は無視
		if (obj.get() == this) { continue; }
		if (obj->GetTag() == TAG_Player) { continue; }

		//// 砲弾の時は大砲は対象外に
		//if (this->GetGimmickTag() == GIMMICK_TAG::TAG_Shell){
		//	if(obj->GetGimmickTag() == GIMMICK_TAG::TAG_Cannon)
		//		continue;
		//}	

		SpherResult spherResult;

		// 球による当たり判定
		if (obj->HitCheckBySpherVsMesh(info, spherResult))
		{
			// 初めに当たった物を基準とする (初回ヒット)
				finalSpherResult = spherResult;

			if (spherResult.m_push.LengthSquared() < finalSpherResult.m_push.LengthSquared()) {
				finalSpherResult = spherResult;

				hitObj = obj;
			}
		}
	}

	if (finalSpherResult.m_hit) {
		//this->m_mWorld.SetTranslation(this->m_mWorld.GetTranslation() + finalSpherResult.m_push);
		
		this->m_mWorld.Move(finalSpherResult.m_push);

		//this->m_mWorld.GetTranslation() += finalSpherResult.m_push;
	}

	m_isBump = finalSpherResult.m_hit;

	//if (this->GetGimmickTag() == GIMMICK_TAG::TAG_Shell) {
	//	if (finalSpherResult.m_hit)
	//		//if(hitObj == nullptr)
	//		this->Destroy();
	//}
}

void GameObject::CheckBump(Matrix& rMat)
{
	if (this->GetName_str() == "StageMap") {
		return;
	}
	if (this->GetTag() == TAG_Player) {
		return;
	}

	// 球情報の作成
	SphereInfo info;

	// 中心点 = キャラクターの位置
	info.m_pos = rMat.GetTranslation();

	// 判定範囲　※サイズが変わるオブジェクトは拡大率に応じてサイズ変更
	if (this->GetTag() == TAG_MoveObject) {
		//info.m_radius = 1.5f * ((rMat.GetScale().x * rMat.GetScale().y * rMat.GetScale().z) / 3.0f);		// キャラの大きさに合わせて半径のサイズもいい感じに設定する
		info.m_radius = 1.5f * (((rMat.GetScale().x * rMat.GetScale().y * rMat.GetScale().z) / 3.0f) * 1.1);		// キャラの大きさに合わせて半径のサイズもいい感じに設定する

		if (info.m_radius <= 0.1) {
			info.m_radius = 0.1f;
		}
	}
	else {
		info.m_radius = 0.7f;
	}

	// 確認のため判定場所をデバッグ表示
	//Scene::GetInstance().AddDebugSphereLine(info.m_pos, info.m_radius, DirectX::SimpleMath::Color{ 1,0,0,1 });

	// 当たったキャラクター
	std::shared_ptr<GameObject> hitObj = nullptr;

	SpherResult finalSpherResult;

	// 全てのオブジェクトと球面判定をする
	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		// 自分自身は無視
		if (obj.get() == this) { continue; }
		if (obj->GetTag() == TAG_Player) { continue; }

		SpherResult spherResult;

		// 球による当たり判定
		if (obj->HitCheckBySpherVsMesh(info, spherResult))
		{
			// 初めに当たった物を基準とする (初回ヒット)
			finalSpherResult = spherResult;

			if (spherResult.m_push.LengthSquared() < finalSpherResult.m_push.LengthSquared()) {
				finalSpherResult = spherResult;

				hitObj = obj;
			}
		}
	}

	if (finalSpherResult.m_hit) {
		rMat.Move(finalSpherResult.m_push);
		//this->m_mWorld.Move(finalSpherResult.m_push);
	}
}

// オブジェクト作成 (class名を元に生成)
std::shared_ptr<GameObject> CreateGameObject(const std::string& name)
{
	// オブジェクト
	if (name == "GameObject") {
		return std::make_shared<GameObject>();
	}

	// 自機
	if (name == "Human")
	{
		return std::make_shared<Human>();
	}

	// リフト
	if (name == "Lift")
	{
		return std::make_shared<Lift>();
	}

	// 移動可能オブジェクト
	if (name == "Object")
	{
		return std::make_shared<Object>();
	}

	// Switch
	if (name == "Gimmick")
	{
		return std::make_shared<Gimmick>();
	}

	// アクションゲーム画面
	if (name == "ActionGameProcess")
	{
		return std::make_shared<ActionGameProcess>();
	}

	// 文字列が既存のクラスに一致しなかった
	assert(0 && "存在しないGameObjectクラスです！");

	return nullptr;
}