#pragma once

//===========================================
//
// 便利機能
//
//===========================================

// 算術系短縮名
namespace Math = DirectX::SimpleMath;

// 角度変換
constexpr float ToRadians = (3.141592654f / 180.0f);
constexpr float ToDegrees = (180.0f / 3.141592654f);

// 安全にReleaseするための関数
template<class T>
void SafeRelease(T*& p)
{
	if (p)
	{
		p->Release();
		p = nullptr;
	}
}

// 安全にDeleteするための関数
template<class T>
void SafeDelete(T*& p)
{
	if (p)
	{
		delete p;
		p = nullptr;
	}
}

//===========================================
//
// 色定数
//
//===========================================
static const Math::Color	kWhiteColor		= Math::Color(1, 1, 1, 1);
static const Math::Color	kBlackColor		= Math::Color(0, 0, 0, 1);
static const Math::Color	kRedColor		= Math::Color(1, 0, 0, 1);
static const Math::Color	kGreenColor		= Math::Color(0, 1, 0, 1);
static const Math::Color	kBlueColor		= Math::Color(0, 0, 1, 1);


//===========================================
//
// ファイル
//
//===========================================

// ファイルパスから、親ディレクトリまでのパスを取得
inline std::string GetDirFromPath(const std::string &path)
{
	const std::string::size_type pos = std::max<signed>(path.find_last_of('/'), path.find_last_of('\\'));
	return (pos == std::string::npos) ? std::string() : path.substr(0, pos + 1);
}

//===========================================
//
// 文字列関係
//
//===========================================

// std::string版 sprintf
template <typename ... Args>
std::string Format(const std::string& fmt, Args ... args)
{
	size_t len = std::snprintf(nullptr, 0, fmt.c_str(), args ...);
	std::vector<char> buf(len + 1);
	std::snprintf(&buf[0], len + 1, fmt.c_str(), args ...);
	return std::string(&buf[0], &buf[0] + len);
}

//=====================================================
//
// テクスチャ
//
//=====================================================

// viewから画像情報を取得する
void GetTextureInfo(ID3D11View* view, D3D11_TEXTURE2D_DESC& outDesc);

//=====================================================
//
// JSON
//
//=====================================================

void MergePrefab(json11::Json& rSrcJson);