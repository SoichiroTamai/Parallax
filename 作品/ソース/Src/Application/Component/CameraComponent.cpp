#include "CameraComponent.h"

#include "../Component/CameraComponent.h"

// コンストラクター
CameraComponent::CameraComponent(GameObject& owner) : m_owner(owner)
{
	// 視野角を設定
	m_viewingAngle = 90.0f;

	m_near	= 0.01f;
	m_far	= 5000.0f;

	// 視錐生成 + 視野角等の設定
	m_mProj.CreateProjection_PerspectiveFov(90 * ToRadians,
		D3D.GetZBuffer()->GetAspectRatio(), 0.01f, 5000.0f);

	// カメラの座標修正 (注視点はそのままでどれだけ離れた位置にあるか)　※ 注視点の設定 Human::Update()内 　trans.CreatTranslation にて設定
	//m_mOffset._41 += 1.0f;		// x座標
	//m_mOffset._42 += 1.0f;		// y座標
	m_mOffset._43 += 0.4f;		// z座標
}

// デコンストラクター
CameraComponent::~CameraComponent()
{
}

// カメラ行列・ビュー行列設定（ 行列 m と 行列 Offset が合成され、最終てきなカメラ行列になる）
void CameraComponent::SetCameraMatrix(const Matrix& m)
{
	// カメラ行列をセット
	m_mCam = m_mOffset * m;

	// カメラ行列からビュー行列を算出
	m_mView = m_mCam;
	m_mView.Inverse();
}

// カメラ情報（ビュー・射影行列など）をシェーダーにセット
void CameraComponent::SetToShader()
{
	// 追従カメラ座標をシェーダーにセット
	SHADER.m_cb7_Camera.Work().CamPos = m_mCam.GetTranslation();

	// 追従カメラビュー行列をシェーダーにセット
	SHADER.m_cb7_Camera.Work().mV = m_mView;

	// 追従カメラの射影行列をシェーダーにセット
	SHADER.m_cb7_Camera.Work().mP = m_mProj;

	// カメラ情報（ ビュー行列、射影行列 ）を、シェーダーの定数バッファへセット
	SHADER.m_cb7_Camera.Write();
}

void CameraComponent::CreateRayInfoFormPlanPos(RayInfo& rDstInfo, const Math::Vector2 planePos)
{
	// コンポポーネントの中にある行列を逆行列計算
	Matrix invView = m_mView;
	invView.Inverse();
	Matrix invPrj = m_mProj;
	invPrj.Inverse();

	// 画面の幅、高さの取得
	int screenW = D3D.GetBackBuffer()->GetWidth();
	int screenH = D3D.GetBackBuffer()->GetHeight();

	// ビューポート行列の計算
	Matrix invViewport;
	invViewport._11 = screenW * 0.5f;
	invViewport._22 = -screenH * 0.5f;
	invViewport._41 = screenW * 0.5f;
	invViewport._42 = screenH * 0.5f;

	// ビューポート行列も逆行列化
	invViewport.Inverse();

	// 2D → 3D　座標変換行列作成
	Matrix convertMat = invViewport * invPrj * invView;

	// 最近接点の座標作成
	Vec3 nearPos(planePos.x, planePos.y, 0.0f);
	Vec3 farPos(planePos.x, planePos.y, 1.0f);

	// 3D座標へ変換する
	nearPos.TransformCoord(convertMat);
	farPos.TransformCoord(convertMat);

	// 最近点と最遠点からレイ情報を作成
	rDstInfo.m_pos = nearPos;							// 発射地点は一番手前から
	rDstInfo.m_dir = farPos - nearPos;					// 発射方向は一番手前から一番奥に向かって
	rDstInfo.m_maxRange = rDstInfo.m_dir.Length();		// 判定距離は最近接点と最遠点の長さ分
	rDstInfo.m_dir.Normalize();							// 方向ベクトルは単位化を忘れずに
}
/*
void CameraComponent::UpdateRey()
{
	// レーザーの終点を求める
	Vec3 mStartPos = { m_mCam.GetTranslation().x, m_mCam.GetTranslation().y , m_mCam.GetTranslation().z };

	laserStart = mStartPos;
	laserEnd = {};
	laserDir = m_mCam.GetAxisZ();

	laserDir.Normalize();	// 拡大が入っていると1以上になるので正規化

	laserDir *= m_laserMaxRange;	//レーザーの射程方向ベクトルを伸ばす

	laserEnd = laserStart + laserDir;	// レーザーの終点は発射ベクトル＋レーザーの長さ分

	//Scene::GetInstance().AddDebugLine(laserStart, laserEnd, { 0.0f,1.0,1.0f,1.0f });
}
*/