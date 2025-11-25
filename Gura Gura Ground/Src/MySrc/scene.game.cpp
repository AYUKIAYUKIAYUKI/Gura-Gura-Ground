//============================================================================
// 
// ゲームシーン [scene.game.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "scene.game.h"

// 遷移先のシーン
#include "API.scene.manager.h"
#include "scene.title.h"

// インプット取得のため
#include "API.input.manager.h"

// オブジェクト生成・破棄のため
#include "API.renderer.h"
#include "API.object.manager.h"
#include "player.h"
#include "gimmick.manager.h"

//****************************************************
// デバッグ用
//****************************************************
namespace
{
	// 定数
	const int   NUM_PLAYER = 4;
	const float INIT_DIST  = 10.0f;

	// グローバル
	OBJ::Transform g_TF = { { 0.5f, 0.5f, 0.5f }, {0.0f, 0.0f, 0.0f, 1.0f}, {-INIT_DIST, 25.0f, -INIT_DIST} };
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSceneGame::CSceneGame()
{
	// プレイヤーの生成
	for (unsigned char wIdxPlayer = 0; wIdxPlayer < NUM_PLAYER; ++wIdxPlayer)
	{
		// 良い感じに四方に散らばらせる
		if (wIdxPlayer % 2 == 0) g_TF.Pos.z *= -1.0f;
		if (wIdxPlayer % 2 == 1) g_TF.Pos.x *= -1.0f;

		CObject::Create<CPlayer>(
			[&wIdxPlayer](CPlayer* p) -> bool
			{
				p->SetIdxPlayer(wIdxPlayer);
				p->SetTransform(g_TF);
				p->FactoryRigidBody(1.0f, 1.0f, 1.0f);
				return true;
			},
			OBJ::TYPE::PLAYER);
	}

	//ギミックマネージャーの生成
	CGimmickManager::RefInstance();
}

//============================================================================
// デストラクタ
//============================================================================
CSceneGame::~CSceneGame()
{}

//============================================================================
// 更新処理
//============================================================================
void CSceneGame::Update()
{
	// シーン変更
	if (CInputManager::RefInstance().GetTrackerKeyboard().pressed.Enter)
	{
		Change();
	}

	// シーンの更新
	CGimmickManager::RefInstance().Update();
}

//============================================================================
// シーン変更
//============================================================================
void CSceneGame::Change()
{
	// 全オブジェクトに死亡フラグを立てる
	CObjectManager::RefInstance().SetDeathAllObject();

	// タイトルシーンへ
	CSceneManager::RefInstance().ChangeScene(std::make_unique<CSceneTitle>());
}