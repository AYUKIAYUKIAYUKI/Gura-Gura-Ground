//============================================================================
// 
// ゲームシーン、ヘッダファイル [scene.game.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.scene.h"
#include "obstacle_editer.h"

//****************************************************
// 前方宣言
//****************************************************
class CHudCount;
class CHud;
class CPlayer;
class CSymbol;

//****************************************************
// ゲームクラスの定義
//****************************************************
class CSceneGame final : public CScene
{
	//****************************************************
	// 静的定数の定義
	//****************************************************
	static const int MAX_COUNT  = 4; // 最大カウント数 
	static const int MAX_PLYAER = 4; // 最大プレイヤー数

public:

	//****************************************************
	// special function
	//****************************************************
	CSceneGame();           // デフォルトコンストラクタ
	~CSceneGame() override; // デストラクタ

	//****************************************************
	// function
	//****************************************************
	void Update() override; // 更新処理
	void Change() override; // シーン変更

private:

	//****************************************************
	// function
	//****************************************************
	void SpawnHUD();     // HUDスポーン
	void SpawnField();   // フィールドスポーン
	void SpawnPlayer();  // プレイヤースポーン
	void SpawnCPU();     // CPUスポーン
	void SpawnSymbol();  // シンボルスポーン
	void SetHudCount();  // HUD：カウントセット
	void SetSymbol();    // シンボルセット
	void SetStartGame(); // ゲーム開始セット
	bool CheckGameSet(); // ゲームセットチェック

	//****************************************************
	// data
	//****************************************************

	// HUD
	std::array<CHudCount*, MAX_COUNT> m_apHudCount;  // HUD：カウント
	CHud*                             m_pHudFinish;  // HUD：フィニッシュ
	bool                              m_bStart;      // 開始状態
	int                               m_nStartCount; // 開始カウント
	bool                              m_bFinish;     // 終了状態

	// プレイヤーの弱参照配列
	std::array<std::weak_ptr<CPlayer>, MAX_PLYAER> m_apwPlayers;

	// シンボルの配列
	std::array<CSymbol*, MAX_PLYAER> m_apSymbol; // シンボル：プレイヤー用
	std::vector<CSymbol*>            m_vpSymbol; // シンボル：エネミー用

	// 障害物エディター
	ObstacleEditer m_ObstacleEditer;
};