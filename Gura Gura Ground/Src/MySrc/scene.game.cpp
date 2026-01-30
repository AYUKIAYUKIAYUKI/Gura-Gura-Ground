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
#include "API.sound.manager.h"

/* 仮 */
#include "API.texture.manager.h"
#include <windfield.h>
#include "field.ice.h"


std::vector<float> times;

int CSceneGame::s_nHumanPlayerNum = {};
int CSceneGame::s_nCPUNum = {};

//****************************************************
// プリプロセッサディレクティブ
//****************************************************
#define ENDLESS_BATTLE 0

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

	static bool g_bUseCPU = true;

#if 0
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
#endif

#if ENDLESS_BATTLE
	void GameSceneUnkoOshiko()
	{
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		if (ImGui::TreeNode("[ GameScene ]"))
		{
			ImGui::Checkbox("CPU Enable", &g_bUseCPU);
			ImGui::TreePop();
		}
		ImGui::Separator();
		ImGui::End();
	}
#endif
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSceneGame::CSceneGame(int nRS)
	: m_pHudFinish(nullptr)
	, m_bStart(false)
	, m_nStartCount(0)
	, m_bFinish(false)
	, m_ObstacleEditer{}
	, m_nRS(nRS)
{
	// カメラの初期設定
	CCamera* pCamera = CRenderer::RefInstance().GetCamera();
	pCamera->SetPos({ 0.0f, 2.0f, 0.0f });
	pCamera->SetPosTarget({ 0.0f, 2.0f, 0.0f });
	pCamera->SetRot({ -0.75f, 0.0f, 0.0f });
	pCamera->SetRotTarget({ -0.75f, 0.0f, 0.0f });
	pCamera->SetDistanceTarget(35.0f);

	// HUDスポーン
	SpawnHUD();

	// フィールドスポーン
	SpawnField();

	// 障害物エディターの初期化
	m_ObstacleEditer.LoadParams("Data\\JSON\\obscale_table.json"); //障害物パラメーターを読み込む
	g_LastUpdateTime = std::chrono::steady_clock::now(); //現在の時間に合わせる
	g_GameTime = 0.0f;

	s_nHumanPlayerNum = CInputManager::RefInstance().GetConnectedGamePadNum();
	s_nCPUNum = MAX_PLYAER - s_nHumanPlayerNum;

	CPlayer::s_vSurvivalTimes.resize(s_nHumanPlayerNum);
	std::fill(CPlayer::s_vSurvivalTimes.begin(), CPlayer::s_vSurvivalTimes.end(), 0.0f);

	CEnemyPlayer::s_vSurvivalTimes.resize(s_nCPUNum);
	std::fill(CEnemyPlayer::s_vSurvivalTimes.begin(), CEnemyPlayer::s_vSurvivalTimes.end(), 0.0f);
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
		if (!m_bBGMStart)
		{
			CSoundManger::RefInstance().Play("BGM_STAGE_NORMAL", false, 0.0f, 1.5f);

			m_bBGMStart = true;
		}

		//プレイモード中の自動スポーン処理
		m_ObstacleEditer.PlayModeSpawn(deltaTime);
	}

#if ENDLESS_BATTLE
	GameSceneUnkoOshiko();
#endif // ENDLESS_BATTLE

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

#ifndef NDEBUG
	if (CInputManager::RefInstance().GetTrackerKeyboard().pressed.R)
	{
		Change();
	}
#endif
}

//============================================================================
// シーン変更
//============================================================================
void CSceneGame::Change()
{
#if ENDLESS_BATTLE
	// 全オブジェクトに死亡フラグを立てる
	CObjectManager::RefInstance().SetDeathAll();

	// エフェクトを全て停止
	CEffectManager::RefInstance().StopAll();

	// ゲームシーンをリセットして再度シーンを設定
	CSceneManager::RefInstance().ChangeScene(std::make_unique<CSceneGame>());
#else
	// 全オブジェクトに死亡フラグを立てる
	CObjectManager::RefInstance().SetDeathAll();

	//ステージBGMを止める
	CSoundManger::RefInstance().Stop("BGM_STAGE_NORMAL");

	// エフェクトを全て停止
	CEffectManager::RefInstance().StopAll();

	// カメラ制御の終了処理
	CCameraController::RefInstance().Finalize();

	std::vector<float> times;
	for (int i = 0; i < CSceneGame::s_nHumanPlayerNum; ++i) times.push_back(CPlayer::s_vSurvivalTimes[i]);
	for (int i = 0; i < CSceneGame::s_nCPUNum; ++i) times.push_back(CEnemyPlayer::s_vSurvivalTimes[i]);
	auto resultScene = std::make_unique<CSceneResult>(times, CSceneGame::s_nHumanPlayerNum, CSceneGame::s_nCPUNum);

	//生存時間を渡しつつ、画面遷移
	CSceneManager::RefInstance().ChangeScene(std::move(resultScene));
#endif
}

//============================================================================
// ゲーム開始セット
//============================================================================
void CSceneGame::SetStartGame()
{
	// プレイヤースポーン
	SpawnPlayer();

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
#if ENDLESS_BATTLE
	m_nStartCount = static_cast<int>(MAX_COUNT + 1);
#else
	m_nStartCount = static_cast<int>(g_GameTime);
#endif

	// カウントが進んだ時だけ判定
	if (m_prevPlayCount != m_nStartCount) 
	{
		if (m_nStartCount <= 2 && m_nStartCount > -1) 
		{
			CSoundManger::RefInstance().Play("CntDown", false, 0.0f, 1.0f);
		}
		else if (m_nStartCount == 3) 
		{
			CSoundManger::RefInstance().Play("CntStart", false, 0.0f, 1.0f);
		}
		m_prevPlayCount = m_nStartCount;
	}


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
	if (m_nRS == 0)
	{
		// フィールドの水平方向の大きさ
		const float fSpanField = 15.0f;
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
	else if (m_nRS == 1)
	{
		// フィールドの水平方向の大きさ
		const float fSpanField = 15.0f;
		const float fSpanAdjust = 0.95f;

		// 地面を生成
		CObjectManager::CreateShare<CFieldIce>(
			[&fSpanField, &fSpanAdjust](CFieldIce* p) -> bool
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

	// コライダーのスパン
	const float fColliderSpan = 0.5f;

	// コントローラーの接続数を取得
	unsigned char wConnectedPadNum = CInputManager::RefInstance().GetConnectedGamePadNum();

	int cpuIdx = 0;

#if ENDLESS_BATTLE
	unsigned char wTotalPlayerNum = MAX_PLYAER;
	if (!g_bUseCPU)
	{
		wTotalPlayerNum = 1;
	}
	for (unsigned char wPlayerIndex = 0; wPlayerIndex < wTotalPlayerNum; ++wPlayerIndex)
#else
	for (unsigned char wPlayerIndex = 0; wPlayerIndex < MAX_PLYAER; ++wPlayerIndex)
#endif
	{
		// 良い感じに四方に散らばらせる
		if (wPlayerIndex % 2 == 0) PlayersInitTransform.Pos.z *= -1.0f;
		if (wPlayerIndex % 2 == 1) PlayersInitTransform.Pos.x *= -1.0f;

		if (wPlayerIndex < wConnectedPadNum)
		{
			// プレイヤー生成
			const std::shared_ptr<CPlayer>& spPlayer = CObjectManager::CreateShare<CPlayer>(
				[wPlayerIndex, &PlayersInitTransform, fColliderSpan](CPlayer* p) -> bool
				{
					// プレイヤーインデックスを決定
					p->SetIdxPlayer(wPlayerIndex);

					// トランスフォームの設定
					p->SetTransform(PlayersInitTransform);

					// コライダーの生成
					p->FactoryCollider(fColliderSpan, fColliderSpan, fColliderSpan);

					return true;
				},
				OBJ::TYPE::PLAYER);

			// ゲームシーン用のプレイヤーの弱参照を保有
			m_apwPlayers[wPlayerIndex] = spPlayer;
		}
		else
		{
			// キャプチャリストでcpuIdxをキャプチャ（値/参照どちらでもOK。ここではコピー）
			std::shared_ptr<CEnemyPlayer> spCPU = CObjectManager::CreateShare<CEnemyPlayer>(
				[cpuIdx, &PlayersInitTransform, fColliderSpan](CEnemyPlayer* p) -> bool
				{
					p->SetIdxCPU(cpuIdx);
					p->SetTransform(PlayersInitTransform);
					p->FactoryCollider(fColliderSpan, fColliderSpan, fColliderSpan);
					return true;
				},
				OBJ::TYPE::CPU);

			m_vpwCPUs.push_back(spCPU);

			m_vpSymbol.push_back(CObjectManager::CreateRaw<CSymbol>(
				[](CSymbol* pSymbol) -> bool
				{
					pSymbol->SetSymbolIdx(MAX_PLYAER);
					return true;
				}));
			cpuIdx++;
		}
	}

	// CPUの数を取得
	unsigned char wEnemyNum = static_cast<unsigned char>(m_vpwCPUs.size());

	// 保有しているCPUを使用して設定
	for (unsigned char i = 0; i < wEnemyNum; ++i)
	{
		for (unsigned char j = 0; j < wEnemyNum; ++j)
		{
			// 自分自身はスキップ
			if (i == j) continue;

			m_vpwCPUs[i].lock()->searchEnemy(m_vpwCPUs[j].lock());
		}
	}

	// シンボルスポーン
	SpawnSymbol();
}

//============================================================================
// シンボルスポーン
//============================================================================
void CSceneGame::SpawnSymbol()
{
	// コントローラーの接続数を取得
	unsigned char wConnectedPadNum = CInputManager::RefInstance().GetConnectedGamePadNum();

	for (unsigned char wPlayerIndex = 0; wPlayerIndex < wConnectedPadNum; ++wPlayerIndex)
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
	unsigned char validPlayers = (unsigned char)s_nHumanPlayerNum; //もしくはm_apSymbol.size()でも
	for (unsigned char wIdx = 0; wIdx < validPlayers; ++wIdx)
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

	// CPUのインデックス用
	unsigned char wIdxCPU = 0;

	// CPUの弱参照配列を走査
	for (const auto& rItCPU : m_vpwCPUs)
	{
		if (std::shared_ptr<CEnemyPlayer> spCPU = rItCPU.lock())
		{
			// シンボルのトランスフォームの取得
			OBJ::Transform SymbolTransform = m_vpSymbol[wIdxCPU]->GetTransform();

			// シンボルの位置をプレイヤーの位置に合わせる
			SymbolTransform.Pos = spCPU->GetTransform().Pos;
			SymbolTransform.Pos.y += m_vpSymbol[wIdxCPU]->GetSymbolOffsetY();

			// シンボルのトランスフォームを設定
			m_vpSymbol[wIdxCPU]->SetTransform(SymbolTransform);
		}
		else
		{
			// プレイヤーが存在しなかったらシンボルを消す
			if (m_vpSymbol[wIdxCPU])
			{
				m_vpSymbol[wIdxCPU]->SetDeath();
			}
		}

		++wIdxCPU;
	}
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