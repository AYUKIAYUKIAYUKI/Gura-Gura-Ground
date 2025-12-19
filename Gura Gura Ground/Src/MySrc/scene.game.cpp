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

/* 一次生成 */
#include "ball.h"
#include "bar.h"
#include "bomb.h"
#include "cameracontroller.h"
#include "tornado.h"


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
		const auto& rPlayerList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);

		// 一体もプレイヤーが存在しないなら (本当はそうでは無い)
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
	// コリジョン描画の切り替え
	CCollider::SwitchRenderCollision();

	// 初期設定
	CCameraController::RefInstance().Initialize();

	// 地面を生成
	float fSpanField = 15.0f;
	CObjectManager::CreateShare<CField>(
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

		auto spPlayer = CObjectManager::CreateShare<CPlayer>(
			[&i](CPlayer* p) -> bool
			{
				p->SetIdxPlayer(i);
				p->SetTransform(g_BoxTF);
				p->FactoryCollider(1.0f, 1.0f, 1.0f);
				return true;
			},
			OBJ::TYPE::PLAYER);

		// プレイヤー登録
		CCameraController::RefInstance().Regist(spPlayer.get());
	}

m_ObstacleEditer.LoadParams("Data\\JSON\\obscale_table.json"); //障害物パラメーターを読み込む
	g_LastUpdateTime = std::chrono::steady_clock::now(); //現在の時間に合わせる
	g_GameTime = 0.0f;	// 障害物スポーンメニュー表示
	m_ObstacleEditer.EditerMenu();

	// スポーン時間プリセットメニュー表示
	m_ObstacleEditer.SpawnTimePresetEditor();

	//プレイモード中の自動スポーン処理
	m_ObstacleEditer.PlayModeSpawn(deltaTime);
	CCameraController::RefInstance().Update();	// ゲームセットしたらシーン遷移

	c++;
	if (c > 300
		&& !a)
	{
		float fSpanField = 15.0f;

		// 竜巻の生成
		CObjectManager::CreateRaw<CTornado>(
			[&fSpanField](CTornado* p) -> bool
			{
				float Size = 3.0f;
				float Pos = fSpanField + 5.0f;
				OBJ::Transform TF = p->GetTransform();
				TF.Pos = { -Pos, 0.0f, Pos };
				p->SetTransform(TF);
				p->SetStartPos(TF.Pos);
				p->FactoryCollider(Size, Size * 0.5f, Size);
				p->SetDepth(Pos * 2.0f);
				p->SetWidth(Pos * 2.0f);
				return true;
			},
			OBJ::TYPE::OBSTACLE);

		a = true;
	}

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
	//// 全オブジェクトに死亡フラグを立てる
	//CObjectManager::RefInstance().SetDeathAll();
	//// タイトルシーンへ
	//CSceneManager::RefInstance().ChangeScene(std::make_unique<CSceneTitle>());
}