#pragma once

class CameraComponent;
class InputComponent;
class ModelComponent;

struct SphereInfo;
struct RayInfo;

struct SpherResult;

// オブジェクト識別用 (タグ変数)
enum OBJECT_TAG {
	TAG_None		= 0x00000000,	// 属性なし：初期設定用
	TAG_Character	= 0x00000001,	// キャラクター設定
	TAG_Player		= 0x00000002,	// プレイヤー属性
	TAG_StageObject = 0x00000004,	// 背景オブジェクト属性
	TAG_MoveObject  = 0x00000008,	// 移動可能オブジェクト属性
	TAG_AttackHit	= 0x00000010,	// 攻撃が当たる属性
	TAG_Gimmick     = 0x00000020,	// ギミック属性
};

// ギミック識別用タグ ( enum が 複数回使用出来なかった為 namespace を使用)
// ※enum時エラー内容： 重大度レベル	コード	説明	プロジェクト	ファイル	行	抑制状態エラー	C2365	'TAG_None': 再定義; 以前の定義は '列挙子' でした。(ソース ファイルをコンパイルしています Src\Application\Component\ModelComponent.cpp)	Project	D : \3DProject\Program\BaseFramework\Src\Application\Game\GameObject.h	26
namespace GIMMICK_TAG {
	const unsigned int TAG_None			= 0;		// 属性なし
	const unsigned int TAG_Switch		= 1;		// スイッチ
	const unsigned int TAG_CreateBox	= 1 << 1;	// Box生成
	const unsigned int TAG_Move			= 1 << 2;	// 移動
	const unsigned int TAG_PowerSpring	= 1 << 3;	// ばね
	const unsigned int TAG_Cannon		= 1 << 4;	// 大砲
	const unsigned int TAG_Shell		= 1 << 5;	// 砲弾
}

// thisポインターをシェアードポインターに変更する事に許可する
class GameObject : public std::enable_shared_from_this<GameObject>
{
public:
	GameObject();
	virtual ~GameObject();

	virtual void Deserialize(const json11::Json& jsonObj);	// 初期化：オブジェクト生成用外部データの解放
	virtual void Update();		// 更新
	virtual void Draw();		// 描画

	//inline const std::string& Getname() const { return m_name; }

	// 半透明物の描画
	virtual void DrawEffect() {}

	// 2D描画
	virtual void Draw2D() {};

	// シャドウマップを描画
	virtual void DrawShadowMap();

	// Imgui更新
	virtual void ImGuiUpdate();

	// ゲッター, セッター
	inline const Matrix& GetMatrix()const { return m_mWorld; }		// 座標
	inline void SetMatrix(const Matrix& rMat) { m_mWorld = rMat; }

	inline bool IsAlive() const { return m_alive; }					// 生存フラグ 取得
	inline void Destroy() { m_alive = false; }						//			  OFF

	inline void SetTag(UINT tag) { m_tag = tag; }					// オブジェクト識別タグ
	inline UINT GetTag() const { return m_tag; }

	inline void SetGimmickTag(UINT tag) { m_gimmick_tag = tag; }	// ギミック識別タグ
	inline UINT GetGimmickTag() const { return m_gimmick_tag; }

	inline const char* GetName() const {				// "Name"取得
		return m_name.c_str();
	}

	inline const std::string GetName_str() const {		// // "Name"取得 (文字列比較用) 例：if(obj->GetName_str() == "Name" )
		return std::string(GetName());
	}

	//inline const char* GetNameS() const {		// strings型用　使用例 : if(GetNameS() == "ObjectName")
	//	return m_name.c_str() + '\0';
	//}

	// カメラコンポーネント取得
	std::shared_ptr<CameraComponent>	GetCameraComponent() { return m_spCameraComponent; }
	// 入力コンポーネント取得
	std::shared_ptr<InputComponent>		GetInputComponent() { return m_spInputComponent; }
	// モデルコンポーネント取得
	std::shared_ptr<ModelComponent>		GetModelComponent() { return m_spModelComponent; }

	const Matrix& GetPrevMatrix() { return m_mPrev; }

	// このキャラクターが動いた分の行列を取得
	Matrix GetOneMove() {
		Matrix mPI = m_mPrev;
		mPI.Inverse();				// 動く前の逆行列
		return mPI * m_mWorld;		// 動く前の逆行列 * 今の行列 = 一回動いた分の行列
	}

	// 球による当たり判定
	bool HitCheckBySphere(const SphereInfo& rInfo);

	// レイによる当たり判定
	bool HitCheckByRay(const RayInfo& rInfo, RayResult& rResult);

	// 球による当たり判定 (メッシュ)
	bool HitCheckBySpherVsMesh(const SphereInfo& rInfo, SpherResult& rResult);
	
	// 重力加算
	void UpdateGravity();
	
	// 当たり判定全般
	virtual void UpdateCollision();					// 当たり判定全般
	
	// 地面との判定
	virtual bool CheckGround(float& rDstDistance);	// 地面との判定
	
	// 押し戻し処理
	virtual void CheckBump();		// ぶつかり判定 (横)
	void CheckBump(Matrix& rMat);	// 指定した座標でのぶつかり判定

	static const float s_allowToStepHeight;		// 歩いて乗り越えられる段差の高さ
	static const float s_landingHeight;			// 地面から足が離れていても着地していると判定する高さ (坂道などを下るときに宙に浮くのを避ける)

	//// 下ギミックフラグ切り替え
	//void SetGimmickBelowFlg(bool flg) { m_GimmickBelowFlg = flg; }
	//void SetGimmick1FrameFlg(bool flg) { m_Gimmick1FrameFlg = flg; }

	// ノードデータ取得
	ModelInfo::Node* GetNodeData(std::string NodeName);

	// ノードを動かす
	void MoveNode(std::string NodeName, Vec3 Movement) {
		this->GetNodeData(NodeName)->m_localTransform.Move(Movement);
	}

	//// サブセット (マテリアルを設定)
	//void SetNodeSubset(std::string NodeName,UINT subsetNum)
	//{
	//	//this->GetNodeData(NodeName)->m_spMesh->GetSubsets()[0].MaterialNo = subsetNum;

	//	//this->GetNodeData(NodeName)->m_spMesh->DrawSubset(subsetNum);
	//}

	bool GetIsBump() { return this->m_isBump; }
	
	bool GetIsGround() { return this->m_isGround; }

	Vec3 GetForce() { return m_force; }
	void SetForce(Vec3 moment) { m_force = moment; }

	std::shared_ptr<GameObject> GetHitUnderObj() { return m_hitUnderObj; }

	//void MoveObj(Vec3 move) { this->m_force += move; }
	void MoveObj(Vec3& move) { this->m_mWorld.Move(move); }

	void SetGimmickType(int GimmickType) { NowAction_GimmickType = GimmickType; }
	int GetGimmickType() { return NowAction_GimmickType; }

	// 軸修正 (求めた新しい軸を前方向にセットする)
	void SetWorldAxisZ(Vec3& v) { m_mWorld.SetAxisZ(v); }

protected:

	//virtual void Release();		// 解放

	// カメラコンポーネント
	std::shared_ptr<CameraComponent> m_spCameraComponent = std::make_shared<CameraComponent>(*this);
	// インプットコンポーネント
	std::shared_ptr<InputComponent> m_spInputComponent = std::make_shared<InputComponent>(*this);
	// モデルコンポーネント
	std::shared_ptr<ModelComponent> m_spModelComponent = std::make_shared<ModelComponent>(*this);

	Matrix	m_mWorld;		// ゲーム内の絶対領域
	Matrix	m_mPrev;		// 動く前の行列
	Vec3	m_mFirstPos;	// 初期座標

	bool	m_alive = true;
	UINT	m_tag		  = OBJECT_TAG::TAG_None;	// オブジェクト用
	UINT	m_gimmick_tag = GIMMICK_TAG::TAG_None;	// ギミック用

	std::string m_name = "GameObject";	// 名前

	float	m_colRadius = 2.0f;	// このキャラクターの半径

	float	m_gravity = 0.01f;	// 重力の強さ
	bool	m_isGround;			// 着地しているかどうか	
	Vec3	m_force;			// キャラクターにかかる移動させる力 (落下、跳躍、移動)

	//bool	m_GimmickBelowFlg = false;	// 下にあるオブジェクトかギミックかどうか ( true → ギミック, false → その他 )
	bool	m_gimmickMoveFlg = false;	// ギミック連続稼働防止

	std::shared_ptr<GameObject> m_hitUnderObj = nullptr;

	int NowAction_GimmickType = GIMMICK_TAG::TAG_None;	// 今実行されているギミック(ばね用)

	bool m_isBump;
};

// クラス名からGameObjectを作成
std::shared_ptr<GameObject> CreateGameObject(const std::string& name);

// 球判定に使うデータ
struct SphereInfo
{
	Vec3 m_pos = {};
	float m_radius = 0.0f;

};

// レイ判定用データ
struct RayInfo
{
	Vec3	m_pos;	// レイ(光線)の発射場所
	Vec3	m_dir;	// レイの発射方向
	float	m_maxRange = 0.0f;	// レイが届く最大距離
};

// 球面判定の結果データ
struct SpherResult
{
	Vec3	m_push;				// 押し戻すベクトル
	bool	m_hit = false;		// ヒットしたか
};