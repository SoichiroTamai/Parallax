#pragma once

// レイ判定をした時の結果情報
struct RayResult
{
	float	m_distance = FLT_MAX;	// 当たったところまでの距離
	bool	m_hit = false;		// 当たったかどうか

	Vec3 rHitPos = {};			// ヒットした場所
	Vec3 rHitBeforePos = {};	// ヒットした手前の座標 (オブジェクト生成用)
};

// レイによる当たり判定
bool RayToMesh(
	const DirectX::XMVECTOR& rRayPos,
	const DirectX::XMVECTOR& rRayDir,
	float maxDistancs, const Mesh& rMesh,
	const Matrix& rMatrix,
	RayResult& rResult
);

//球対メッシュの当たり判定 (点と三角形の距離が半径以下で当たった)
bool SphereToMesh(
	const Math::Vector3& rSherePos,		// 球の中心点の場所
	float radius,						// 球の半径
	const Mesh& meth,					// 判定するメッシュ情報
	const DirectX::XMMATRIX& matrix,	// 判定する相手の行列
	Math::Vector3& rPusthedPos			// 当たっていた場合、押し返された球の中心点
);

// 点 vs 三角形との最近接点を求める
// ・p			… 点の座標
// ・a			… 三角形の点１
// ・b			… 三角形の点２
// ・c			… 三角形の点３
// ・outPt		… 最近接点の座標が入る
void PointToTriangle(const DirectX::XMVECTOR& p, const DirectX::XMVECTOR& a,
	const DirectX::XMVECTOR& b, const DirectX::XMVECTOR& c, DirectX::XMVECTOR& outPt);
