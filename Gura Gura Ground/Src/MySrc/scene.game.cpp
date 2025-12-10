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
#include "API.object.manager.h"
#include "field.h"
#include "player.h"
#include "API.world.h"

/* 一次生成 */
#include "ball.h"
#include "bar.h"

#include "obstacle_editer.h"

//****************************************************
// 仮
//****************************************************
namespace
{
	// 定数
	const int nNumB = 4;
	const float fInitDist = 10.0f;

	// オブジェクトの出現方向, 0:縦(上下), 1:横(左右)
	int g_ObstacleDirection = 0;

	bool g_AutoSpawnEnabled = true;

	// 障害物の出現間隔(秒), imguiで設定
	float g_ObstacleSpawnInterval = 3.0f;

	// 前回出現した時刻
	float g_ObstacleLastSpawnTime = 0.0f;

	std::chrono::steady_clock::time_point g_LastUpdateTime;
	float g_GameTime = 0.0f;

	// グローバル
	OBJ::Transform g_BoxTF = { { 0.5f, 0.5f, 0.5f }, {0.0f, 0.0f, 0.0f, 1.0f}, {-fInitDist, 25.0f, -fInitDist} };

	/* シンプルなゲームセット */
	bool GameSet()
	{
		// プレイヤーのリストを取得
		const auto& rPlayerList = CObjectManager::RefInstance().RefObjList(OBJ::TYPE::PLAYER);

		// 一体もプレイヤーが存在しないなら
		if (rPlayerList.size() < 1)
		{
			return true;
		}

		return false;
	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSceneGame::CSceneGame()
{
	CCollider::SwitchRenderCollision();

	// 地面を生成
	float fSpanField = 15.0f;
	CObject::Create<CField>(
		[&fSpanField](CField* p) -> bool
		{
			p->SetTransform(
				{
					{ fSpanField, 1.0f, fSpanField },
					{ 0.0f, 0.0f, 0.0f, 1.0f },
					{ 0.0f, 5.0f, 0.0f }
				}
			);

			p->FactoryCollider(fSpanField * 2.0f, 1.0f * 2.0f, fSpanField * 2.0f);
			return true;
		},
		OBJ::TYPE::FIELD);

	// ああ
	const float fUnkoSpan = 3.0f;

	// プレイヤーの生成
	for (unsigned char i = 0; i < nNumB; ++i)
	{
		// 良い感じに四方に散らばらせる
		if (i % 2 == 0) g_BoxTF.Pos.z *= -1.0f;
		if (i % 2 == 1) g_BoxTF.Pos.x *= -1.0f;

		CObject::Create<CPlayer>(
			[&i](CPlayer* p) -> bool
			{
				p->SetIdxPlayer(i);
				p->SetTransform(g_BoxTF);
				p->FactoryCollider(1.0f, 1.0f, 1.0f);
				return true;
			},
			OBJ::TYPE::PLAYER);
	}

	ObstacleEditer::LoadParams("Data\\JSON\\obscale_table.json");
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
	// 障害物スポーンメニュー表示
	ObstacleEditer::ShowEditerMenu();

	// 自動スポーン判定
	ObstacleEditer::TryAutoSpawn(g_GameTime);

	// ゲームセットしたらシーン遷移
	if (GameSet())
	{
		Change();
	}

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