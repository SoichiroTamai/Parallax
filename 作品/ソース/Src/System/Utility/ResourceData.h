#pragma once

/*
	const  … 変更不可
	static … 変更可能
*/

// シーンデータ
namespace SceneData
{
	const std::string GamePlayScreen_FileName = "Data/Scene/ActionGame.json";
}

// マップデータ
namespace MapData
{
	// マップ一覧
	const std::string EditorStage_FileName			= "Data/Scene/StageMap/Stage_for_Editing.json";	// 全オブジェクトが配置されているマップ
	const std::string GimmickStage_FileName			= "Data/Scene/StageMap/Stage_Gimmick.json";		// 全ギミックを使用するマップ
	const std::string Stage1_FileName				= "Data/Scene/StageMap/Stage1.json";			// ステージ1
	const std::string PresentationStage_FileName	= "Data/Scene/StageMap/Stage_Presentation.json";			// プレゼン用ステージ

	// 最初に読み込むマップ
	const std::string GameFirstStage_FileName = PresentationStage_FileName;
}

// 動かせるノード名 ( 各ステージごとに指定 )　※ ノード名は Blender 上の名前を参照
namespace MoveNodeNameData
{
	// EditorStage---------------------------------------------


	// GimmickStage ------------------------------------------
	const std::string MoveWall_01	= "MoveWall_01";		// 開始時正面にある壁
	const std::string MoveFloor_01	= "MoveFloor_01";		// 上下に動く床

	const std::string Switch_01_Wall = "MoveObj_switch_01";	// スイッチ01を押すと開く壁
	const std::string Switch_02_CannonCeiling = "MoveObj_switch_02_CannonCeiling";	// スイッチ01を押すと開く壁

	// Stage1 -------------------------------------------------

}

// オブジェクト情報格納用
namespace ObjectResourceData
{
	const std::string CreateBoxJsonFileName = "Data/Scene/GimmickObject/CreateBox.json";		// 画面中央に生成するオブジェクト (デフォルト)				
	const std::string MoveBoxFileName = "Data/Scene/GimmickObject/MoveBox.json";				// 乗ると移動するオブジェクト				
	const std::string PowerSpringJsonFileName = "Data/Scene/GimmickObject/PowerSpring.json";	// ばね				
	const std::string ShellJsonFileName = "Data/Scene/GimmickObject/Shell.json";				// 砲弾

	static std::string CreateObjectJsonFileName = CreateBoxJsonFileName;		// 画面中央に生成するオブジェクト
}

// スイッチ情報 (1ビットずつ使用)
namespace SwitchData
{
	const unsigned int None = 0;

	const unsigned int Switch_01 = 1;
	const std::string SwitchName_01 = "Switch_01";


	const unsigned int Switch_02 = 1 << 1;
	const std::string SwitchName_02 = "Switch_02";

	//static unsigned int NowSwitchData = None;
}

//class SwitchData
//{
//public:
//	static SwitchData& GetInstance()
//	{
//		static SwitchData instance;
//		return instance;
//	}
//
//	~SwitchData() {};
//
//	static unsigned int NowSwitchData;
//
//	const unsigned int Switch_01 = 1;
//	const std::string SwitchName_01 = "Switch_01";
//
//
//	const unsigned int Switch_02 = 1 << 1;
//};