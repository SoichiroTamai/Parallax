#pragma once

// ==================================
// リソース管理クラス
// ・デザインパターンのFlyweightパターンを採用
// モデルを1度しか読み込まないようにする
// ※ 射出するたびモデルを読み込み停止するのを防ぐため 
// ==================================

class ResouceFactory
{
public:
	// モデルデータ取得
	std::shared_ptr<ModelInfo> GetModel(const std::string& filename);

	// テクスチャデータ取得
	std::shared_ptr<Texture> GetTexture(const std::string& filename);

	// JSON取得
	json11::Json GetJSON(const std::string& filename);


	// 管理を破棄する
	void Clear()
	{
		m_modelMap.clear();
		m_texMap.clear();
		m_jsonMap.clear();
	}

private:

	// モデルデータ管理マップ	連想配列
	std::unordered_map<std::string, std::shared_ptr<ModelInfo>> m_modelMap;

	// テクスチャ管理マップ
	std::unordered_map<std::string, std::shared_ptr<Texture>> m_texMap;

	// JSON管理マップ
	std::unordered_map<std::string, json11::Json>  m_jsonMap;

	// JSON読み込み
	json11::Json LoadJSON(const std::string& filename);

	// ==================================
	// シングルトン
	// ==================================
private:
	ResouceFactory() {}
public:
	static ResouceFactory& GetInstance()
	{
		static ResouceFactory instance;
		return instance;
	}
};

#define ResFac ResouceFactory::GetInstance()