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
#include "API.renderer.h"

// 遷移先のシーン
#include "API.scene.manager.h"
#include "scene.result.h"

// インプット取得のため
#include "API.input.manager.h"

// オブジェクト生成・破棄のため
#include "API.object.manager.h"
#include "effect.manager.h"
#include "hud.count.h"
#include "field.h"
#include "player.h"
#include "symbol.h"
#include <enemy1.h>

// イベント処理のため
#include "cameracontroller.h"

/* 仮 */
#include "API.texture.manager.h"


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

	// あああ
	void ModifyModelOffset(CField* pField)
	{
		// モデルオフセットの取得
		DirectX::XMFLOAT3 raa = pField->GetModelOffset();

		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		ImGui::DragFloat("Pos X", &raa.x, 0.01f);
		ImGui::DragFloat("Pos Y", &raa.y, 0.01f);
		ImGui::DragFloat("Pos Z", &raa.z, 0.01f);
		ImGui::End();

		// モデルオフセットの設定
		pField->SetModelOffset(raa);
	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSceneGame::CSceneGame()
	: m_pHudFinish(nullptr)
	, m_bStart(false)
	, m_nStartCount(0)
	, m_bFinish(false)
	, m_ObstacleEditer{}
{
	/* コリジョン描画の切り替え */
	//CCollider::SwitchRenderCollision();

	// HUDスポーン
	SpawnHUD();

	// フィールドスポーン
	SpawnField();

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

#ifndef NDEBUG
	// 障害物スポーンメニュー表示
	m_ObstacleEditer.EditerMenu();

	// スポーン時間プリセットメニュー表示
	m_ObstacleEditer.SpawnTimePresetEditor();
#endif

	if (m_bStart)
	{
		//プレイモード中の自動スポーン処理
		m_ObstacleEditer.PlayModeSpawn(deltaTime);
	}

	// HUD：カウントセット
	SetHudCount();

	if (m_bStart)
	{
		// カメラコントローラーの更新
		CCameraController::RefInstance().Update();

		// シンボルセット
		SetSymbol();

		// ゲームセットしたらシーン遷移
		/* ゲームセットチェック */
		if (CheckGameSet())
		{
			/* 即シーン変更 */
			Change();
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

	// エフェクトを全て停止
	CEffectManager::RefInstance().StopAll();

	//生存時間
	std::vector<float> times = CPlayer::s_vSurvivalTimes;
	auto resultScene = std::make_unique<CSceneResult>(times);

	//遷移時に生存時間も渡す
	CSceneManager::RefInstance().ChangeScene(std::move(resultScene));
}
//============================================================================
// ゲーム開始セット
//============================================================================
void CSceneGame::SetStartGame()
{
	// プレイヤースポーン
	SpawnPlayer();

	// CPUスポーン


	// シンボルスポーン
	SpawnSymbol();

	// カメラコントローラーの初期化
	CCameraController::RefInstance().Initialize();
}

//============================================================================
// HUDスポーン
//============================================================================
void CSceneGame::SpawnHUD()
{
	// HUD：カウントを生成
	for (unsigned char wIdx = 0; wIdx < MAX_COUNT; ++wIdx)
	{
		if (!m_apHudCount[wIdx])
		{
			m_apHudCount[wIdx] = CObjectManager::CreateRaw<CHudCount>(
				[&wIdx](CHudCount* p)  -> bool
				{
					// 補間係数を変更
					p->SetLerpPower(0.1f);

					// カウント数のインデックス設定
					p->SetHudCountIdx(wIdx);

					return true;
				},
				OBJ::TYPE::NONE,
				OBJ::LAYER::DEFAULT);
		}
	}

	// HUD：フィニッシュを生成
	if (!m_pHudFinish)
	{
		// トランスフォームの設定を一切行いません
		m_pHudFinish = CObjectManager::CreateRaw<CHud>(
			[](CHud* p)  -> bool
			{
				/* テクスチャの設定 */
				p->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Human.D"));

				return true;
			},
			OBJ::TYPE::NONE,
			OBJ::LAYER::DEFAULT);
	}
}

//============================================================================
// HUD：カウントセット	
//============================================================================
void CSceneGame::SetHudCount()
{
	// ゲーム開始していたら何もしない
	if (m_bStart)
	{
		return;
	}

	/* カウンターがあったので、つかわしてもらいます */
	m_nStartCount = static_cast<int>(g_GameTime);

	// 現在のカウント数と設定済みのインデックスで自動表示
	for (const auto& rIt : m_apHudCount)
	{
		rIt->SetNowCount(static_cast<unsigned char>(m_nStartCount));
	}

	// カウントの最大値を超えたら開始フラグを立てる
	if (m_nStartCount > MAX_COUNT)
	{
		m_bStart = true;

		SetStartGame();
	}
}

//============================================================================
// フィールドスポーン
//============================================================================
void CSceneGame::SpawnField()
{
	// フィールドの水平方向の大きさ
	const float fSpanField  = 15.0f;
	const float fSpanAdjust = 0.95f;

	// 地面を生成
	CObjectManager::CreateShare<CField>(
		[&fSpanField, &fSpanAdjust](CField* p) -> bool
		{
			// トランスフォームの設定
			p->SetTransform(
				{
					{ fSpanField * fSpanAdjust, fSpanField, fSpanField * fSpanAdjust },
					{ 0.0f, 0.0f, 0.0f, 1.0f },
					{ 0.0f, 5.0f, 0.0f }
				});

			// コライダーの生成
			p->FactoryCollider(fSpanField, 1.0f, fSpanField);

			return true;
		},
		OBJ::TYPE::FIELD,
		OBJ::LAYER::BG);
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
				p->FactoryCollider(0.5f, 0.5f, 0.5f);

				return true;
			},
			OBJ::TYPE::PLAYER);

		// プレイヤーの弱参照を作成
		m_apwPlayers[wPlayerIndex] = spPlayer;
	}
}

//============================================================================
// CPUスポーン
//============================================================================
void CSceneGame::SpawnCPU()
{
	const float fSize = 0.5f;
	const float EnemyIndex = 2;
	float Posx = -5.0f;
	std::vector<std::shared_ptr<CEnemyPlayer>>pEnemy;

	for (unsigned char wPlayerIndex = 0; wPlayerIndex < EnemyIndex; ++wPlayerIndex)
	{
		// プレイヤーの初期トランスフォーム
		OBJ::Transform PlayersInitTransform = {
			{ fSize, fSize, fSize },
			{ 0.0f, 0.0f, 0.0f, 1.0f},
			{ Posx, 15.0f, -5.0f }
		};

		// 敵生成
		pEnemy.push_back(CObjectManager::CreateShare<CEnemyPlayer>(
			[PlayersInitTransform, fSize](CEnemyPlayer* p) -> bool
			{
				// トランスフォームの設定
				p->SetTransform(PlayersInitTransform);

				// コライダーの生成
				p->FactoryCollider(fSize, fSize, fSize);

				return true;
			},
			OBJ::TYPE::NONE) //TYPEはENEMYとか別枠で確保した}
		);

		Posx = 5.0f;
	}

	for (int i = 0; i < EnemyIndex; ++i)
	{
		for (int j = 0; j < EnemyIndex; ++j)
		{
			if (i == j) continue;  // 自分自身はスキップ
			pEnemy[i]->searchEnemy(pEnemy[j]);
		}
	}
	
	// シンボル生成
	m_vpSymbol.push_back(CObjectManager::CreateRaw<CSymbol>(
		[](CSymbol* pSymbol) -> bool
		{
			// シンボルのインデックス設定
			pSymbol->SetSymbolIdx(MAX_PLYAER);

			return true;
		}));
}

//============================================================================
// シンボルスポーン
//============================================================================
void CSceneGame::SpawnSymbol()
{
	for (unsigned char wPlayerIndex = 0; wPlayerIndex < MAX_PLYAER; ++wPlayerIndex)
	{
		// シンボル生成
		m_apSymbol[wPlayerIndex] = CObjectManager::CreateRaw<CSymbol>(
			[&wPlayerIndex](CSymbol* pSymbol) -> bool
			{
				// シンボルのインデックス設定
				pSymbol->SetSymbolIdx(wPlayerIndex);

				return true;
			});
	}
}

//============================================================================
// シンボルセット
//============================================================================
void CSceneGame::SetSymbol()
{
	// プレイヤーの弱参照配列を走査
	for (unsigned char wIdx = 0; wIdx < MAX_PLYAER; ++wIdx)
	{
		// プレイヤーが存在していたら
		if (std::shared_ptr<CPlayer> spPlayer = m_apwPlayers[wIdx].lock())
		{
			// シンボルのトランスフォームの取得
			OBJ::Transform SymbolTransform = m_apSymbol[wIdx]->GetTransform();

			// シンボルの位置をプレイヤーの位置に合わせる
			SymbolTransform.Pos = spPlayer->GetTransform().Pos;
			SymbolTransform.Pos.y += m_apSymbol[wIdx]->GetSymbolOffsetY();

			// シンボルのトランスフォームを設定
			m_apSymbol[wIdx]->SetTransform(SymbolTransform);
		}
		else
		{
			// プレイヤーが存在しなかったらシンボルを消す
			if (m_apSymbol[wIdx])
			{
				m_apSymbol[wIdx]->SetDeath();
			}
		}
	}

#if 0 // aCP
	for (auto& pSymbol : m_vpSymbol)
	{
		// シンボルのトランスフォームの取得
		OBJ::Transform SymbolTransform = pSymbol->GetTransform();

		// シンボルの位置を敵の位置に合わせる
		SymbolTransform.Pos = g_pEnemy->GetTransform().Pos;
		SymbolTransform.Pos.y += pSymbol->GetSymbolOffsetY();

		// シンボルのトランスフォームを設定
		pSymbol->SetTransform(SymbolTransform);
	}
#endif
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