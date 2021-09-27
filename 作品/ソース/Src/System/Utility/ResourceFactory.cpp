#include "ResourceFactory.h"

std::shared_ptr<ModelInfo> ResouceFactory::GetModel(const std::string& filename)
{
	// filename のモデルが有るか？
	auto itFund = m_modelMap.find(filename);

	// ない場合
	if (itFund == m_modelMap.end())
	{
		// 生成 & 読み込み
		auto newModel = std::make_shared<ModelInfo>();
		if (newModel->Load(filename) == false)
		{
			// 読み込み失敗時は、nullを返す
			return nullptr;
		}
		// リスト(map)に登録
		m_modelMap[filename] = newModel;
		// リソースを返す
		return newModel;
	}
	// ある場合
	else
	{
		// 用意されているものを返す
		return (*itFund).second;	// (*itFund).first … filename, std::string		 (*itFund).second … newModel, std::shared_ptr<ModelInfo>
	}
}

std::shared_ptr<Texture> ResouceFactory::GetTexture(const std::string& filename)
{
	// filenameのテクスチャーがあるか？
	auto itFound = m_texMap.find(filename);
	// 無い場合
	if (itFound == m_texMap.end())
	{
		// 生成 & 読み込み
		auto newTexture = std::make_shared<Texture>();
		if (newTexture->Load(filename) == false) {
			// 読み込み失敗時は、nullを返す
			return nullptr;
		}
		// リスト(map)に登録
		m_texMap[filename] = newTexture;
		// リソースを返す
		return newTexture;
	}
	// ある場合
	else
	{
		return (*itFound).second;
	}
}

json11::Json ResouceFactory::GetJSON(const std::string& filename)
{
	// filename のモデルが有るか？
	auto itFund = m_jsonMap.find(filename);

	// ない場合
	if (itFund == m_jsonMap.end())
	{
		// JSONファイルを文字列として読み込む
		json11::Json json = LoadJSON(filename);
		if (json.is_null())
		{
			assert(0 && "[GetJSON]jsonファイルが見つからない");

			// 読み込み失敗時は、nullを返す
			return nullptr;
		}
		// リスト(map)に登録
		m_jsonMap[filename] = json;
		// リソースを返す
		return json;
	}
	// ある場合
	else
	{
		// 用意されているものを返す
		return (*itFund).second;	// (*itFund).first … filename, std::string		 (*itFund).second … newModel, std::shared_ptr<ModelInfo>
	}
}

json11::Json ResouceFactory::LoadJSON(const std::string& filename)
{
	// jsonファイルを開く
	std::ifstream ifs(filename);
	if (ifs.fail())
	{
		assert(0 && "jsonファイルのパスが間違っています");
		return nullptr;
	}

	// 文字列として全読み込み
	std::string strJson((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

	// 文字列のJSONを解析(パース)する
	std::string err;
	json11::Json jsonObj = json11::Json::parse(strJson, err);
	if (err.size() > 0)
	{
		assert(0 && "読み込んだファイルのJson変換に失敗");
		return nullptr;
	}

	return jsonObj;
}