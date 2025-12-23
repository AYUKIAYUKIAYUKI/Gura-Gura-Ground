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
// 仮
// 仮：最終的に必要と判断した変数はメンバに付属してください
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
}

	// グローバル
	OBJ::Transform g_BoxTF = { { 0.5f, 0.5f, 0.5f }, {0.0f, 0.0f, 0.0f, 1.0f}, {-fInitDist, 25.0f, -fInitDist} };
//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSceneGame::CSceneGame()
{
	/* コリジョン描画の切り替え */
	CCollider::SwitchRenderCollision();

	/* シンプルなゲームセット */
	bool GameSet()
	{
		// プレイヤーのリストを取得
		const auto& rPlayerList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);
	// カメラコントローラーの初期化
	CCameraController::RefInstance().Initialize();

		// 一体もプレイヤーが存在しないなら (本当はそうでは無い)
		if (rPlayerList.size() < 1)
		{
			return true;
		}
	// フィールドスポーン
	SpawnField();

		return false;
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
// デフォルトコンストラクタ
// シーン変更
//============================================================================
CSceneGame::CSceneGame()
void CSceneGame::Change()
{
	// コリジョン描画の切り替え
	CCollider::SwitchRenderCollision();
	// 全オブジェクトに死亡フラグを立てる
	CObjectManager::RefInstance().SetDeathAll();

	// 初期設定
	CCameraController::RefInstance().Initialize();
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
	float fSpanField = 15.0f;
	CObjectManager::CreateShare<CField>(
		[&fSpanField](CField* p) -> bool
		{
			// トランスフォームの設定
			p->SetTransform(
				{
					{ fSpanField, 1.0f, fSpanField },
					{ 0.0f, 0.0f, 0.0f, 1.0f },
					{ 0.0f, 5.0f, 0.0f }
				}
			);
				});

			// コライダーの生成
			p->FactoryCollider(fSpanField * 2.0f, 1.0f * 2.0f, fSpanField * 2.0f);

			return true;
		},
		OBJ::TYPE::FIELD);
}

	// ああ
	const float fUnkoSpan = 3.0f;
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

	// プレイヤーの生成
	for (unsigned char i = 0; i < nNumB; ++i)
	for (unsigned char wPlayerIndex = 0; wPlayerIndex < MAX_PLYAER; ++wPlayerIndex)
	{
		// 良い感じに四方に散らばらせる
		if (i % 2 == 0) g_BoxTF.Pos.z *= -1.0f;
		if (i % 2 == 1) g_BoxTF.Pos.x *= -1.0f;
		if (wPlayerIndex % 2 == 0) PlayersInitTransform.Pos.z *= -1.0f;
		if (wPlayerIndex % 2 == 1) PlayersInitTransform.Pos.x *= -1.0f;

		auto spPlayer = CObjectManager::CreateShare<CPlayer>(
			[&i](CPlayer* p) -> bool
		// プレイヤー生成
		const std::shared_ptr<CPlayer>& spPlayer = CObjectManager::CreateShare<CPlayer>(
			[&PlayersInitTransform, wPlayerIndex](CPlayer* p) -> bool
			{
				p->SetIdxPlayer(i);
				p->SetTransform(g_BoxTF);
				// プレイヤーインデックスを決定
				p->SetIdxPlayer(wPlayerIndex);

				// トランスフォームの設定
				p->SetTransform(PlayersInitTransform);

				// コライダー生の成
				p->FactoryCollider(1.0f, 1.0f, 1.0f);

				return true;
			},
			OBJ::TYPE::PLAYER);

		// プレイヤー登録
		// プレイヤーの弱参照を作成
		m_apwPlayers[wPlayerIndex] = spPlayer;

		// カメラコントローラーへプレイヤーを登録
		/* 出来れば生ポインタでなく弱参照を用いてください */
		CCameraController::RefInstance().Regist(spPlayer.get());
	}
}

	m_ObstacleEditer.LoadParams("Data\\JSON\\obscale_table.json"); //障害物パラメーターを読み込む
	g_LastUpdateTime = std::chrono::steady_clock::now(); //現在の時間に合わせる
	g_GameTime = 0.0f;
//============================================================================
// CPUスポーン
//============================================================================
void CSceneGame::SpawnCPU()
{
	// 敵の各辺のスパン
	const float fSize = 1.0f;


	// 敵生成
	float fSize = 1.0f;

	CObjectManager::CreateRaw<CEnemy1>(
		[&fSize](CEnemy1* p) -> bool
		[fSize](CEnemy1* p) -> bool
		{
			// トランスフォームの設定
			p->SetTransform(
				{
					{ fSize, fSize, fSize },
					{ 0.0f, 0.0f, 0.0f, 1.0f },
					{ 2.0f, 15.0f, 2.0f }
				}
			);
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
// デストラクタ
// ゲームセットチェック
//============================================================================
CSceneGame::~CSceneGame()
{}

//============================================================================
// 更新処理
//============================================================================
void CSceneGame::Update()
bool CSceneGame::CheckGameSet()
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
	if (GameSet())
	for (const auto& wpPlayer : m_apwPlayers)
















	{
		Change();
		/* プレイヤーが1人でも生きていたらゲーム継続 */
		if (!wpPlayer.expired())
		{
			return false;
		}
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
	return true;










}