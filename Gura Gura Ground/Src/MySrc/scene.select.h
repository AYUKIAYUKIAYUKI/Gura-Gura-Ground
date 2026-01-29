//============================================================================
// 
// セレクトシーン、ヘッダファイル [select.scene.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.scene.h"

//****************************************************
// 前方宣言
//****************************************************
class CBeamLight;
class CHud;

class CPhysicsModel;

//****************************************************
// セレクトシーンクラスの定義
//****************************************************
class CSceneSelect final : public CScene
{
	//****************************************************
	// エイリアスを定義
	//****************************************************
	using BeamLightQue = std::pair<float, DirectX::XMFLOAT2>;

	//****************************************************
	// 静的定数を定義
	//****************************************************
	static constexpr unsigned char MAX_PLAYER = 4;

public:

	//****************************************************
	// special function
	//****************************************************
	CSceneSelect();           // デフォルトコンストラクタ
	~CSceneSelect() override; // デストラクタ

	//****************************************************
	// function
	//****************************************************
	void Update() override; // 更新処理
	void Change() override; // シーン変更

private:

	//****************************************************
	// function
	//****************************************************
	void WhileEvent_QueInstantiateLight(); // ビームライトの生成キュー
	void WhileEvent_CennectCheck();        // 接続チェック
	void SpawnFromIdx(unsigned char wIdx); // インデックスからの生成
	void DownCutter();                     // 斬首
	void Alignment();                      // 整列

	//****************************************************
	// data
	//****************************************************

	/* 仮 */
	int                       m_nCommonCnt;    // 汎用カウンター
	std::vector<CBeamLight*>  m_vpBeamLight;   // ビームライト
	std::vector<BeamLightQue> m_vBeamLightQue; // ビームライトのためのキュー

	/* さらに */
	std::vector<CHud*>          m_vpSymbol;     // シンボル
	std::vector<CPhysicsModel*> m_vpPM;         // モデル
	std::vector<CPhysicsModel*> m_vpBody;       // ボディ
	std::vector<CPhysicsModel*> m_vpCutter;     // カッター
	std::vector<CPhysicsModel*> m_vpGuillotion; // ギロチン台
	bool                        m_bDeathPenaly; // 死刑フラグ
	bool                        m_bSelectStart; // 選択開始
};