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
#include <enemy1.h>

// イベント処理のため
#include "cameracontroller.h"

//****************************************************
// 仮：最終的に必要と判断した変数はメンバに付属してください
//****************************************************
namespace
{
	// 定数
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
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSceneGame::CSceneGame()
{
	/* コリジョン描画の切り替え */
	CCollider::SwitchRenderCollision();

	// カメラコントローラーの初期化
	CCameraController::RefInstance().Initialize();

	// フィールドスポーン
	SpawnField();

	// プレイヤースポーン
	SpawnPlayer();

	// CPUスポーン
	SpawnCPU();

	// 障害物エディターの初期化
	m_ObstacleEditer.LoadParams("Data\\JSON\\obscale_table.json"); //障害物パラメーターを読み込む
	g_LastUpdateTime = std::chrono::steady_clock::now(); //現在の時間に合わせる
	g_GameTime = 0.0f;
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
	//タイム計測
	auto now = std::chrono::steady_clock::now();
	float deltaTime = std::chrono::duration<float>(now - g_LastUpdateTime).count();
	g_LastUpdateTime = now;
	g_GameTime += deltaTime;

	// 障害物スポーンメニュー表示
	m_ObstacleEditer.EditerMenu();

	// スポーン時間プリセットメニュー表示
	m_ObstacleEditer.SpawnTimePresetEditor();

	//プレイモード中の自動スポーン処理
	m_ObstacleEditer.PlayModeSpawn(deltaTime);
	CCameraController::RefInstance().Update();	// ゲームセットしたらシーン遷移

	/* ゲームセットチェック */
	if (CheckGameSet())
	{
		/* 即シーン変更 */
		Change();
	}
}

//============================================================================
// シーン変更
//============================================================================
void CSceneGame::Change()
{
	// 全オブジェクトに死亡フラグを立てる
	CObjectManager::RefInstance().SetDeathAll();

	// タイトルシーンへ
	CSceneManager::RefInstance().ChangeScene(std::make_unique<CSceneTitle>());
}

//============================================================================
// フィールドスポーン
//============================================================================
void CSceneGame::SpawnField()
{
	// フィールドの水平方向の大きさ
	const float fSpanField = 15.0f;

	// 地面を生成
	CObjectManager::CreateShare<CField>(
		[&fSpanField](CField* p) -> bool
		{
			// トランスフォームの設定
			p->SetTransform(
				{
					{ fSpanField, 1.0f, fSpanField },
					{ 0.0f, 0.0f, 0.0f, 1.0f },
					{ 0.0f, 5.0f, 0.0f }
				});

			// コライダーの生成
			p->FactoryCollider(fSpanField * 2.0f, 1.0f * 2.0f, fSpanField * 2.0f);

			return true;
		},
		OBJ::TYPE::FIELD);
}

//============================================================================
// プレイヤースポーン
//============================================================================
void CSceneGame::SpawnPlayer()
{
	// プレイヤーの初期トランスフォーム
	OBJ::Transform PlayersInitTransform =
	{
		{ 0.5f, 0.5f, 0.5f },
		{ 0.0f, 0.0f, 0.0f, 1.0f},
		{ -fInitDist, 25.0f, -fInitDist }
	};

	for (unsigned char wPlayerIndex = 0; wPlayerIndex < MAX_PLYAER; ++wPlayerIndex)
	{
		// 良い感じに四方に散らばらせる
		if (wPlayerIndex % 2 == 0) PlayersInitTransform.Pos.z *= -1.0f;
		if (wPlayerIndex % 2 == 1) PlayersInitTransform.Pos.x *= -1.0f;

		// プレイヤー生成
		const std::shared_ptr<CPlayer>& spPlayer = CObjectManager::CreateShare<CPlayer>(
			[&PlayersInitTransform, wPlayerIndex](CPlayer* p) -> bool
			{
				// プレイヤーインデックスを決定
				p->SetIdxPlayer(wPlayerIndex);

				// トランスフォームの設定
				p->SetTransform(PlayersInitTransform);

				// コライダー生の成
				p->FactoryCollider(1.0f, 1.0f, 1.0f);

				return true;
			},
			OBJ::TYPE::PLAYER);

		// プレイヤーの弱参照を作成
		m_apwPlayers[wPlayerIndex] = spPlayer;

		// カメラコントローラーへプレイヤーを登録
		/* 出来れば生ポインタでなく弱参照を用いてください */
		CCameraController::RefInstance().Regist(spPlayer.get());
	}
}

//============================================================================
// CPUスポーン
//============================================================================
void CSceneGame::SpawnCPU()
{
	// 敵の各辺のスパン
	const float fSize = 1.0f;

	// 敵生成
	CObjectManager::CreateRaw<CEnemyPlayer>(
		[fSize](CEnemyPlayer* p) -> bool
		{
			// トランスフォームの設定
			p->SetTransform(
				{
					{ fSize, fSize, fSize },
					{ 0.0f,  0.0f,  0.0f, 1.0f },
					{ 2.0f,  15.0f, 2.0f }
				});

			// コライダーの生成
			p->FactoryCollider(fSize, fSize, fSize);

			return true;
		},
		OBJ::TYPE::NONE); //TYPEはENEMYとか別枠で確保した}
}

//============================================================================
// ゲームセットチェック
//============================================================================
bool CSceneGame::CheckGameSet()
{
	for (const auto& wpPlayer : m_apwPlayers)
	{
		/* プレイヤーが1人でも生きていたらゲーム継続 */
		if (!wpPlayer.expired())
		{
			return false;
		}
	}

	return true;
}