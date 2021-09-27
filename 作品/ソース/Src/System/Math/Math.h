#pragma once

// 3Dベクトル	: 継承
class Vec3 : public DirectX::XMFLOAT3
{
public:
	// 指定行列でVectorを変換する
	Vec3& TransformCoord(const DirectX::XMMATRIX& m)
	{
		*this = XMVector3TransformCoord(*this, m);
		return *this;
	}

	// 指定(回転)行列でVectorを変換する (回転のみ)
	Vec3& TransformNormal(const DirectX::XMMATRIX& m)
	{
		*this = XMVector3TransformNormal(*this, m);
		return *this;
	}

	// デフォルトコンストラクタは座標の0クリアを行う
	Vec3()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	// 座標指定付きのコンストラクタ
	Vec3(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	// XMVECTORから代入してきた時
	Vec3(const DirectX::XMVECTOR& v)
	{
		// 変換して代入
		DirectX::XMStoreFloat3(this, v);

		// 下記と同意だが、上記のSIMD命令を使用した方が高速
		// x = v.m128_f32[0];	// データサイズ_どう使っているか（ マルチプルデータ ）
		// y = v.m128_f32[1];
		// z = v.m128_f32[2];
	}

	// XMFLOAT3から代入してきた時
	Vec3(const DirectX::XMFLOAT3& V)
	{
		x = V.x;
		y = V.y;
		z = V.z;
	}

	// XMVECTORへ変換
	operator DirectX::XMVECTOR() const { return DirectX::XMLoadFloat3(this); }		// const … この関数内でメンバ変数の変更不可

	// Math::Vector3と互換性を持つための関数
	operator Math::Vector3& () { return *(Math::Vector3*)this; }

	// 算術演算子 乗算(*)			// このクラス内で *= のときはこの処理を実行する
	Vec3& operator *= (float s)
	{
		*this = DirectX::XMVectorScale(*this, s);
		return *this;
	}

	Vec3& operator += (Vec3& V1)
	{
		this->x += V1.x;
		this->y += V1.y;
		this->z += V1.z;

		return *this;
	}

	// ベクトル同士の比較
	bool isNE (Vec3& V1)
	{
		if (this->x != V1.x) { return true; }
		if (this->y != V1.y) { return true; }
		if (this->z != V1.z) { return true; }

		return false;
	}

	// 自分を正規化
	Vec3& Normalize()
	{
		*this = DirectX::XMVector3Normalize(*this);
		return *this;
	}

	// 長さを取得
	float Length() const
	{
		return DirectX::XMVector3Length(*this).m128_f32[0];
	}

	// 長さの2乗（高速なので判定用に使用されることが多い）
	float LengthSquared() const
	{
		return DirectX::XMVector3LengthSq(*this).m128_f32[0];
	}

	// 内積 ( イメージ：影の長さ )
	static float Dot(const Vec3& v1, const Vec3& v2)
	{
		return DirectX::XMVector3Dot(v1, v2).m128_f32[0];	// 角度( -1～1 ) … ( 0～180 )
	}

	// 外積 ( 対象物への回転軸やポリゴンの向き(方線)に使用できる )
	static Vec3 Cross(const Vec3& v1, const Vec3& v2)
	{
		return DirectX::XMVector3Cross(v1, v2);		// 2つの直角となるベクトル
	}

	// 長濱先生課題② <-------------------------------------------------------------
	// 渡されたベクトルへ値分自身を向ける ( Missile::Update()内を参照 )
	inline void Complement(const Vec3& vTo, float rot)
	{
		// 回転軸の作成
		Vec3 vRotAxis = Vec3::Cross(*this, vTo);

		// 0ベクトルなら回転しない
		if (vRotAxis.LengthSquared() == 0) { return; }

		// 自分のZ軸方向ベクトルと自分のターゲットへ向かうベクトルの内積
		float d = Vec3::Dot(*this, vTo);

		// 誤差で -1～1 以外になる可能性大なので、クランプする。(std::clampでも可)
		if (d > 1.0f)d = 1.0f;
		else if (d < -1.0f)d = -1.0f;

		// 自分の前方向ベクトルと自身からTargetへ向かうベクトル間の角度(radian)
		float radian = acos(d);

		// 角度制限 １フレームにつき最大でrot度以上回転しない
		if (radian > rot * ToRadians)
		{
			radian = rot * ToRadians;
		}

		// ※※※※※ radian (ここまでで回転角度が求まった) ※※※※※

		DirectX::XMMATRIX mRot = DirectX::XMMatrixRotationAxis(vRotAxis, radian);

		this->TransformNormal(mRot);
	}
	// ---------------------------------------------------------------------------->
};

// 4*4の行列
class Matrix : public DirectX::XMFLOAT4X4
{
public:

	// デフォルトコンストラクタ
	Matrix()
	{
		*this = DirectX::XMMatrixIdentity();
	}

	// XMMATRIXから代入してきた
	Matrix(const DirectX::XMMATRIX& m)
	{
		DirectX::XMStoreFloat4x4(this, m);
	}

	// XMFLOAT4X4,Math::Matrixから代入してきた
	Matrix(const DirectX::XMFLOAT4X4& m)
	{
		memcpy_s(this, sizeof(float) * 16, &m, sizeof(XMFLOAT4X4));
	}

	// XMMATRIXへ変換
	operator DirectX::XMMATRIX() const
	{
		return DirectX::XMLoadFloat4x4(this);
	}

	// Math::Matrixtと互換性を持たせるための関数
	operator Math::Matrix& () { return *(Math::Matrix*)this; }

	// 代入演算子　乗算
	Matrix& operator *= (const Matrix& m)
	{
		*this = DirectX::XMMatrixMultiply(*this, m);	// 第1引数が受け取り側(ベース)の行列、第2引数が送り側の行列
		return *this;
	}

	// 作成 ==========================================================
	// 角度変更 ( 視野用 )
	void RoteteAxis(DirectX::XMVECTOR vec, float angle)
	{
		*this *= DirectX::XMMatrixRotationAxis(vec, angle);
	}

	// 移動行列作成
	void CreateTranslation(float x, float y, float z)
	{
		*this = DirectX::XMMatrixTranslation(x, y, z);
	}

	void CreateTranslation( Vec3& v )
	{
		*this = DirectX::XMMatrixTranslation(v.x, v.y, v.z);
	}

	// X軸回転行列作成
	void CreateRotationX(float angle)
	{
		*this = DirectX::XMMatrixRotationX(angle);
	}

	// 拡縮行列作成
	void CreateScaling(float x, float y, float z)
	{
		*this = DirectX::XMMatrixScaling(x, y, z);
	}

	void CreateScaling(const Vec3& v)
	{
		*this = DirectX::XMMatrixScaling(v.x, v.y, v.z);
	}

	// 指定軸回転行列作成
	void CreateRotationAxis(const Vec3& axis, float angle)
	{
		*this = DirectX::XMMatrixRotationAxis(axis, angle);
	}

	// クォータニオンから回転行列を作成
	void CreatFormQuaternion(const Math::Quaternion& quat)
	{
		*this = DirectX::XMMatrixRotationQuaternion(quat);
	}

	// 透視影行列の作成
	Matrix& CreateProjection_PerspectiveFov(float fovAngleY, float aspectRatio, float nearZ, float farZ)
	{
		*this = DirectX::XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);
		return *this;
	}

	// 正射影行列作成 (近くも遠くも同じ大きさ平行な影が出る影)
	Matrix& CreateProjection_Orthographic(float viewWidth, float viewHeight, float nearZ, float farZ)
	{
		*this = DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ);
		return *this;
	}

	// 操作 ============================================================

	// X軸回転
	void RotateX(float angle)
	{
		*this *= DirectX::XMMatrixRotationX(angle);
	}

	// Y軸回転
	void RotateY(float angle)
	{
		*this *= DirectX::XMMatrixRotationY(angle);
	}

	// Z軸回転
	void RotateZ(float angle)
	{
		*this *= DirectX::XMMatrixRotationZ(angle);
	}

	// 拡縮
	void Scale(float x, float y, float z)
	{
		*this *= DirectX::XMMatrixScaling(x, y, z);
	}

	// 逆行列にする
	void Inverse()
	{
		*this = DirectX::XMMatrixInverse(nullptr, *this);
	}

	// 移動行列
	void Move(const Vec3& v)
	{
		_41 += v.x;
		_42 += v.y;
		_43 += v.z;
	}

	// プロパティ =====================================================
	// X軸を取得 ( 長さ = 拡大率 )
	Vec3 GetAxisX() const { return { _11,_12,_13 }; }

	// Y軸を取得
	Vec3 GetAxisY() const { return { _21,_22,_23 }; }

	// Z軸を取得
	Vec3 GetAxisZ() const { return { _31,_32,_33 }; }

	// X軸をセット
	void SetAxisX(const Vec3& v)
	{
		_11 = v.x;
		_12 = v.y;
		_13 = v.z;
	}

	// Y軸をセット
	void SetAxisY(const Vec3& v)
	{
		_21 = v.x;
		_22 = v.y;
		_23 = v.z;
	}

	// Z軸をセット
	void SetAxisZ(const Vec3& v)
	{
		_31 = v.x;
		_32 = v.y;
		_33 = v.z;
	}

	// 座標をセット
	void SetTranslation(const Vec3& v)
	{
		_41 = v.x;
		_42 = v.y;
		_43 = v.z;
	}

	// XYZの順番で合成したときの、回転角度を算出する
	Vec3 GetAngles() const
	{
		Matrix mat = *this;

		// 各軸を取得
		Vec3 axisX = mat.GetAxisX();
		Vec3 axisY = mat.GetAxisY();
		Vec3 axisZ = mat.GetAxisZ();

		// 各軸を正規化
		axisX.Normalize();
		axisY.Normalize();
		axisZ.Normalize();

		// マトリックスを正規化
		mat.SetAxisX(axisX);
		mat.SetAxisY(axisY);
		mat.SetAxisZ(axisZ);

		Vec3 angles;
		angles.x = atan2f(mat.m[1][2], mat.m[2][2]);
		angles.y = atan2f(-mat.m[0][2], sqrtf(mat.m[1][2] * mat.m[1][2] + mat.m[2][2] * mat.m[2][2]));
		angles.z = atan2f(mat.m[0][1], mat.m[0][0]);

		return angles;
	}

	// Z軸を指定方向に向ける
	Matrix& LookTo(const Vec3& dir, const Vec3& up)
	{
		Vec3 vZ = dir;
		vZ.Normalize();
		Vec3 vX = Vec3::Cross(up, vZ).Normalize();
		if (vX.LengthSquared() == 0)
		{
			vX = { 1,0,0 };
		}
		Vec3 vY = Vec3::Cross(vZ, vX).Normalize();

		float scaleX = GetAxisX().Length();
		float scaleY = GetAxisY().Length();
		float scaleZ = GetAxisZ().Length();

		SetAxisX(vX * scaleX);
		SetAxisY(vY * scaleY);
		SetAxisZ(vZ * scaleZ);

		return *this;
	}


	// 拡大率を指定
	void SetScale(const Vec3& scales)
	{
		this->GetAxisX().Normalize();
		this->GetAxisY().Normalize();
		this->GetAxisZ().Normalize();

		this->SetAxisX(this->GetAxisX() * scales.x);
		this->SetAxisY(this->GetAxisY() * scales.y);
		this->SetAxisZ(this->GetAxisZ() * scales.z);
	}

	// 拡大率取得
	Vec3 GetScale() const
	{
		return{
			this->GetAxisX().Length(),
			this->GetAxisY().Length(),
			this->GetAxisZ().Length()
		};
	}

	// 座標取得
	Vec3 GetTranslation() const { return { _41, _42,_43 }; }

	// 渡された行列に対して正面を向くように回転(ビルボード)する
	inline void SetBillboard(const Matrix& mat)
	{
		// 値取得
		Matrix BillboardMat = mat;

		// インバース ( 対象と逆方向に回転させる為 )
		BillboardMat.Inverse();

		// 座標は自分のものを使う
		Vec3 Pos = this->GetTranslation();

		// 逆行列と合成
		*this *= BillboardMat;

		// 保存しておいた座標を格納
		this->SetTranslation(Pos);
	}
};

// matrix同士の合成
inline Matrix operator * (const Matrix& M1, const Matrix& M2)
{
	using namespace DirectX;
	return XMMatrixMultiply(M1, M2);
}

// =================================
//
// クォータニオン
//
// =================================
class  Quaternion : public DirectX::XMFLOAT4
{
public:
	// Constructorで初期値代入（とりあえず安全を)
	Quaternion()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 1.0f;
	}

	// XMVECTOR から代入してきた時
	Quaternion(const DirectX::XMVECTOR& V)
	{
		// 変換して代入
		DirectX::XMStoreFloat4(this, V);
	}

	// XMVECTORへ変換
	operator DirectX::XMVECTOR() const { return DirectX::XMLoadFloat4(this); }

	// 行列から作成
	Quaternion& CreatFormMatrix(const Math::Matrix& m)
	{
		*this = DirectX::XMQuaternionRotationMatrix(m);
		return *this;
	}

	//// 行列作成
	//XMFLOAT4 CreatFormMatrix (DirectX::BoundingFrustum &mvf, Matrix mat)
	//{
	//	DirectX::XMMATRIX dm;
	//	dm = mat.operator DirectX::XMMATRIX();		// XMMATRIXに変換
	//	DirectX::XMQuaternionRotationMatrix(dm);	// 回転行列から回転クォータニオンを計算

	//	//DirectX::BoundingFrustum mvf;
	//	DirectX::BoundingFrustum::CreateFromMatrix(mvf, dm);	// 指定された射影行列からBoundingFrustumを作成
	//	return mvf.Orientation;									// クォータニオンとして表されるBoundingFrustumの方向
	//}
};