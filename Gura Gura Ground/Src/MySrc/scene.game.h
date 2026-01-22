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
class CEnemyPlayer;
class CSymbol;

//****************************************************
// ゲームクラスの定義
//****************************************************
class CSceneGame final : public CScene
{
	//****************************************************
	// 静的定数の定義
	//****************************************************
	static const int MAX_COUNT = 4; // 最大カウント数 
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

	// シーン初期化 ～
	void SetStartGame(); // ゲーム開始セット
	void SpawnHUD();     // HUDスポーン
	void SetHudCount();  // HUD：カウントセット
	void SpawnField();   // フィールドスポーン
	void SpawnPlayer();  // プレイヤースポーン
	void SpawnSymbol();  // シンボルスポーン

	// ～ ゲーム中 ～
	void SetSymbol();    // シンボルセット
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
	std::array<std::weak_ptr<CPlayer>, MAX_PLYAER> m_apwPlayers; // プレイヤー用
	std::vector<std::weak_ptr<CEnemyPlayer>>       m_vpwCPUs;    // CPU用

	// シンボルの配列
	std::array<CSymbol*, MAX_PLYAER> m_apSymbol; // シンボル：プレイヤー用
	std::vector<CSymbol*>            m_vpSymbol; // シンボル：エネミー用

	static int s_nHumanPlayerNum;  // 人間プレイヤー数
	static int s_nCPUNum;          // CPUプレイヤー数

	// 障害物エディター
	ObstacleEditer m_ObstacleEditer;
};