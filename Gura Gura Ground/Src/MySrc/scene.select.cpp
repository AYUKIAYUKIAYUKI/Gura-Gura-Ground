//============================================================================
// 
// セレクトシーン [scene.select.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "scene.select.h"
#include "API.sound.manager.h"

// 遷移先のシーン
#include "API.scene.manager.h"
#include "scene.game.h"

// インプット取得のため
#include "API.input.manager.h"

// オブジェクト生成・破棄のため
#include "API.renderer.h"
#include "API.HUD.h"
#include "API.object.manager.h"
#include "API.texture.manager.h"
#include "API.fullscreen.2D.h"
#include "beamlight.h"

#include "API.world.h"
#include "API.rigidbody.h"
#include "API.physics.model.h"
#include "API.gltf.manager.h"
#include "symbol.h"

/* デバッグ */
namespace
{
	static float fA = 7.0f;
	static float fB = 5.5f;
	static float fC = 0.0f;

	void AAA()
	{
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");

		const float fStep = 1.0f / 60.0f;

		ImGui::DragFloat("Light A : ", &fA, fStep);
		ImGui::DragFloat("Light B : ", &fB, fStep);
		ImGui::DragFloat("Light C : ", &fC, fStep);

		ImGui::Separator();
		ImGui::End();
	}

	void PrintDebug(int nCnt)
	{
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		ImGui::Text("StopCnt : %d", nCnt);
		ImGui::Separator();
		ImGui::End();
	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSceneSelect::CSceneSelect()
	: m_nCommonCnt(0)
	, m_vpBeamLight{}
	, m_vBeamLightQue{}
	, m_apSymbol{}
	, m_vpPM{}
	, m_vpBody{}
	, m_vpCutter{}
	, m_vpGuillotion{}
	, m_bDeathPenaly(false)
	, m_nStopCnt(0)
	, m_bSelectStart(false)
	, m_apStageHud{}
	, m_nStageIdx{}
	, m_nStageDecide{}
	, m_bStageDecideAll(false)
	, m_nCntChangeStage(0)
	, m_nRandomIdx(0)
	, m_bStageDecided(false)
	, m_bChangeScene(false)
	, m_pHud_CA(nullptr)
	, m_pHud_CB(nullptr)
{
	// サンシャインエフェクトの生成
	auto pfst = CObjectManager::CreateRaw<CFullScreen2D>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
	auto rcpVS = CVertexShaderManager::RefInstance().RefRegistry().BindAtKey("FullScreen.2D");
	pfst->SetVertexShader(rcpVS);
	auto rcpPS = CPixelShaderManager::RefInstance().RefRegistry().BindAtKey("Sunshine");
	pfst->SetPixelShader(rcpPS);
	auto rcpCB = CConstantBufferManager::RefInstance().RefConstantBuffer(static_cast<unsigned char>(CConstantBufferManager::BufferType::UTIL));
	pfst->SetConstantBuffer(rcpCB);

	/* ライト3つ分のキュー */
	const float fOffsetX_Light = 0.3f;
	const float fOffsetY_Light = 0.75f;
	m_vBeamLightQue.push_back({ fA, {  fOffsetX_Light, fOffsetY_Light } });
	m_vBeamLightQue.push_back({ fB, { -fOffsetX_Light, fOffsetY_Light } });
	m_vBeamLightQue.push_back({ fC, {  0.0f,           fOffsetY_Light } });

	// カメラの初期設定
	CCamera* pCamera = CRenderer::RefInstance().GetCamera();
	pCamera->SetPosTarget({ 0.0f, 2.5f, 0.0f });
	pCamera->SetRotTarget({ 0.0f, 0.0f, 0.0f });
	pCamera->SetDistanceTarget(7.5f);

	// HUDの生成 - CA
	m_pHud_CA = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
	m_pHud_CA->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("CA"));

	const float fAA = 2.0f;

	m_pHud_CA->SetTransform(
		{
			{ 586.0f * fAA, 112.0f * fAA, .0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			{ 1980.0f * 0.5f, 900.0f, 0.0f }
		});

	m_pHud_CA->SetTransformTarget(
		{
			{ 586.0f * fAA, 112.0f * fAA, .0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			{ 1980.0f * 0.5f, 900.0f, 0.0f }
		});
}

//============================================================================
// デストラクタ
//============================================================================
CSceneSelect::~CSceneSelect()
{}

//============================================================================
// 更新処理
//============================================================================
void CSceneSelect::Update()
{
	// ビームライトの生成キュー
	WhileEvent_QueInstantiateLight();

	if (!m_bDeathPenaly)
	{
		// 接続チェック
		WhileEvent_CennectCheck();

		// 死刑執行
		CInputManager& rInput = CInputManager::RefInstance();
		if (rInput.GetTrackerGamePad(0).start == DirectX::GamePad::ButtonStateTracker::PRESSED ||
			rInput.GetTrackerGamePad(1).start == DirectX::GamePad::ButtonStateTracker::PRESSED ||
			rInput.GetTrackerGamePad(2).start == DirectX::GamePad::ButtonStateTracker::PRESSED ||
			rInput.GetTrackerGamePad(3).start == DirectX::GamePad::ButtonStateTracker::PRESSED)
		{
			if (m_pHud_CA)
			{
				m_pHud_CA->SetDeath();
			}

			// 死刑執行フラグを立てる
			m_bDeathPenaly = true;

			// ヘッドを転がす
			for (const auto& rIt : m_vpPM)
			{
				if (CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(rIt->GetCollider()))
				{
					// ダイナミック化
					pRigidBody->SetDynamic();

					/* 応急処置 */
					if (btRigidBody* rb = pRigidBody->GetRigidBody())
					{
						rb->setActivationState(DISABLE_DEACTIVATION);
					}
				}
			}

			// ボディを転がす
			for (const auto& rIt : m_vpBody)
			{
				if (CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(rIt->GetCollider()))
				{
					// ダイナミック化
					pRigidBody->SetDynamic();

					/* 応急処置 */
					if (btRigidBody* rb = pRigidBody->GetRigidBody())
					{
						rb->setActivationState(DISABLE_DEACTIVATION);
					}
				}
			}

			// 効果音：アイアン
			CSoundManger::RefInstance().Play("Light", false, -0.5f, 3.0f);
		}
	}
	else
	{
		// 斬首
		DownCutter();

		// ストップカウンターを進める
		++m_nStopCnt;

		// 整列 ～ ステージ選択
		if (m_nStopCnt > 180 && !m_bStageDecideAll)
		{
			if (!m_pHud_CB)
			{
				// HUDの生成 - CB
				m_pHud_CB = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
				m_pHud_CB->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("CB"));

				const float fBB = 2.25f;

				m_pHud_CB->SetTransform(
					{
						{ 505.0f * fBB, 72.0f * fBB, .0f },
						{ 0.0f, 0.0f, 0.0f, 1.0f },
						{ 1980.0f * 0.5f, 900.0f, 0.0f }
					});

				m_pHud_CB->SetTransformTarget(
					{
						{ 505.0f * fBB, 72.0f * fBB, .0f },
						{ 0.0f, 0.0f, 0.0f, 1.0f },
						{ 1980.0f * 0.5f, 900.0f, 0.0f }
					});
			}

			Alignment();
			SpawnSymbol();
			SetSymbol();
			SpawnStageHud();
			SetStageHud();
			SelectStage();
		}

		// ステージ決定
		if (m_bStageDecideAll && !m_bStageDecided)
		{
			/* ああ… */
			Alignment();
			SetSymbol();

			DecideStage();
		}

		// ステージ確定
		if (m_bStageDecided)
		{
			/* ああ… */
			Alignment();
			SetSymbol();

			DecideAppeal();
		}
	}

	// ステージの抽選が完了したらシーン遷移
	if (m_bChangeScene)
	{
		Change();
	}

	// 強制遷移
#ifdef _DEBUG
	if (CInputManager::RefInstance().GetTrackerKeyboard().pressed.Enter)
	{
		Change();
	}
#endif // _DEBUG
}

//============================================================================
// シーン変更
//============================================================================
void CSceneSelect::Change()
{
#if 0
	// 全オブジェクトに死亡フラグを立てる
	CObjectManager::RefInstance().SetDeathAll();

	//タイトルBGMを停止する
	CSoundManger::RefInstance().Stop("BGM_TITLE");

	// ゲームシーンへ
	CSceneManager::RefInstance().ChangeScene(std::make_unique<CSceneGame>());
#else
	// 全オブジェクトに死亡フラグを立てる
	CObjectManager::RefInstance().SetDeathAll();

	//タイトルBGMを停止する
	CSoundManger::RefInstance().Stop("BGM_TITLE");

	auto aa = std::make_unique<CSceneGame>(m_nStageIdx[m_nRandomIdx]);

	//生存時間を渡しつつ、画面遷移
	CSceneManager::RefInstance().ChangeScene(std::move(aa));
#endif
}

//============================================================================
// ビームライトの生成キュー
//============================================================================
void CSceneSelect::WhileEvent_QueInstantiateLight()
{
	const int Size  = static_cast<int>(m_vpBeamLight.size());
	const int nMax  = 3;
	const int nSpan = 15;

	/*	デバッグ */
	if (Size == nMax)
	{
		// リロード
		if (CInputManager::RefInstance().GetStateKeyboard().A)
		{
			// 時間を設定
			m_vpBeamLight[0]->SetTime(fA);
			m_vpBeamLight[1]->SetTime(fB);
			m_vpBeamLight[2]->SetTime(fC);
		}

		// 一斉動作
		if (CInputManager::RefInstance().GetTrackerKeyboard().pressed.Space)
		{
			for (const auto& rIt : m_vpBeamLight) rIt->SetEnableTime();
		}

		/* ままままま */
		++m_nCommonCnt;
		if (m_nCommonCnt > nSpan)
		{
			m_nCommonCnt = 0;
			for (const auto& rIt : m_vpBeamLight) rIt->SetEnableTime();
		}
	}

	// 既定量の生成が済んでいたら処理しない
	if (Size >= nMax)
	{
		return;
	}

	++m_nCommonCnt;

	// 一定のスパンで
	if (m_nCommonCnt > nSpan)
	{
		m_nCommonCnt = 0;

		// ビームライトを生成
		auto pfsbl = CObjectManager::CreateRaw<CBeamLight>(OBJ::TYPE::NONE, OBJ::LAYER::FRONT);

		// 位置設定
		const DirectX::XMFLOAT2& Pos = m_vBeamLightQue[Size].second;
		pfsbl->SetPos(Pos);

		// 時間を設定
		pfsbl->SetTime(m_vBeamLightQue[Size].first);

		// 一応保有
		m_vpBeamLight.push_back(pfsbl);

		// 効果音：アイアン
		CSoundManger::RefInstance().Play("Light", false, -0.5f, 2.0f);
	}
}

//============================================================================
// 接続チェック
//============================================================================
void CSceneSelect::WhileEvent_CennectCheck()
{
	// 現在のパッドの接続数と
	unsigned char wNumPad = CInputManager::RefInstance().GetConnectedGamePadNum();
	unsigned char wSize   = static_cast<unsigned char>(m_vpPM.size());

	if (wNumPad == 0)
	{
		return;
	}
	else if (wNumPad > wSize)
	{
		// 接続数分の生成を試みる
		for (unsigned char wIdx = 0; wIdx < wNumPad; ++wIdx)
		{
			SpawnFromIdx(wIdx);
		}
	}
	else if (wNumPad < wSize)
	{
		m_vpPM[wSize - 1]->SetDeath();
		m_vpPM.pop_back();
	}
}

//============================================================================
// インデックスからの生成
//============================================================================
void CSceneSelect::SpawnFromIdx(unsigned char wIdx)
{
	// glTFモデルのポインタ
	GltfMesh* pHead = nullptr;
	GltfMesh* pBody = nullptr;

	// glTFマネージャーからモデルを取得するキー
	std::string HeadKey = {};
	std::string BodyKey = {};

	switch (wIdx)
	{
	case 0:
		BodyKey = "Guillo_1";
		HeadKey = "Player_1";
		break;

	case 1:
		BodyKey = "Guillo_2";
		HeadKey = "Player_2";
		break;

	case 2:
		BodyKey = "Guillo_3";
		HeadKey = "Player_3";
		break;

	case 3:
		BodyKey = "Guillo_4";
		HeadKey = "Player_4";
		break;

	default:
		break;
	}

	// glTFモデルの取得
	pHead = CGltfManager::RefInstance().RefRegistry().BindAtKey(HeadKey);
	pBody = CGltfManager::RefInstance().RefRegistry().BindAtKey(BodyKey);

	/*--------------------------------------------------------------------------------*/

	// プレイヤー用の初期位置
	DirectX::XMFLOAT3 InitPos = { useful::VEC3_ZERO_INIT };
	const float f = -3.75f + (wIdx * 2.5f);
	InitPos = { f, 1.25f, 0.0f };

	// プレイヤーの初期トランスフォーム
	OBJ::Transform PlayersInitTransform =
	{
		{ 0.5f, 0.5f, 0.5f },
		{ -1.0f, 0.0f, 0.0f, 1.0f},
		InitPos
	};

	// コライダーのスパン
	const float fColliderSpan = 0.5f;

	// ウソのプレイヤー生成
	m_vpPM.push_back(CObjectManager::CreateRaw<CPhysicsModel>(
		[wIdx, &pHead, &PlayersInitTransform, fColliderSpan](CPhysicsModel* p) -> bool
		{
			// モデルのバインド
			p->SetModel(pHead);
			p->SetModelOffset({ 0.0f, 0.0f, 0.0f });

			// ピクセルシェーダ―のバインド
			p->SetPixelShader(CPixelShaderManager::RefInstance().RefRegistry().BindAtKey("Ray.Marching"));

			// トランスフォームの設定
			p->SetTransform(PlayersInitTransform);

			// コライダーの生成
			p->FactoryCollider(fColliderSpan, fColliderSpan, fColliderSpan, Collision::SHAPETYPE::CONE);

			// キネマティックにする
			CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(p->GetCollider());
			pRigidBody->SetKinematic();

			// ワールドトランスフォームを設定
			pRigidBody->SetWorldTransform(PlayersInitTransform);

			return true;
		}));

	/*--------------------------------------------------------------------------------*/

	// ボディの初期トランスフォーム
	OBJ::Transform BodyInitTransform =
	{
		{ 0.5f, 0.5f, 0.5f },
		{ -1.0f, 0.0f, 0.0f, 1.0f },
		{ f, 1.25f, 0.75f }
	};

	// ボディの生成
	m_vpBody.push_back(CObjectManager::CreateRaw<CPhysicsModel>(
		[wIdx, &pBody, &BodyInitTransform, fColliderSpan](CPhysicsModel* p) -> bool
		{
			// モデルのバインド
			p->SetModel(pBody);
			p->SetModelOffset({ 0.0f, 0.0f, 0.0f });

			// ピクセルシェーダ―のバインド
			p->SetPixelShader(CPixelShaderManager::RefInstance().RefRegistry().BindAtKey("Ray.Marching"));

			// トランスフォームの設定
			p->SetTransform(BodyInitTransform);

			// コライダーの生成
			p->FactoryCollider(fColliderSpan, fColliderSpan, fColliderSpan);

			// キネマティックにする
			CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(p->GetCollider());
			pRigidBody->SetKinematic();

			// ワールドトランスフォームを設定
			pRigidBody->SetWorldTransform(BodyInitTransform);

			return true;
		}));

	/*--------------------------------------------------------------------------------*/

	// カッター用の初期位置
	DirectX::XMFLOAT3 CutterInitPos = { InitPos };
	CutterInitPos.x += -0.025f * wIdx;
	CutterInitPos.y += 3.75f;
	CutterInitPos.z += 0.5f;

	// カッターの初期トランスフォーム
	OBJ::Transform CutterInitTransform =
	{
		{ 1.5f, 1.5f, 1.5f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		CutterInitPos
	};

	// カッターの生成
	m_vpCutter.push_back(CObjectManager::CreateRaw<CPhysicsModel>(
		[&InitPos, &CutterInitTransform](CPhysicsModel* p) -> bool
		{
			// モデルのバインド
			p->SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Guillo_Blade"));
			p->SetModelOffset({ 0.0f, 0.0f, 0.0f });

			// トランスフォームの設定
			p->SetTransform(CutterInitTransform);

			// コライダーの生成
			p->FactoryCollider(0.5f, 0.5f, 0.5f, Collision::SHAPETYPE::SPHERE);

			// キネマティックにする
			CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(p->GetCollider());
			pRigidBody->SetKinematic();

			return true;
		}));

	/*--------------------------------------------------------------------------------*/

	// ギロチンの初期トランスフォーム
	OBJ::Transform GuilloInitTransform =
	{
		{ 1.25f, 1.25f, 1.25f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ InitPos.x, 0.0f, 0.5f }
	};

	// ギロチンの生成
	m_vpGuillotion.push_back(CObjectManager::CreateRaw<CPhysicsModel>(
		[&InitPos, &GuilloInitTransform, fColliderSpan](CPhysicsModel* p) -> bool
		{
			// モデルのバインド
			p->SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Guillo_Base"));
			p->SetModelOffset({ 0.0f, 0.0f, 0.0f });

			// トランスフォームの設定
			p->SetTransform(GuilloInitTransform);

			// コライダーの生成
			p->FactoryCollider(fColliderSpan, fColliderSpan, fColliderSpan);

			// 質量を0にする
			CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(p->GetCollider());
			pRigidBody->SetMass(0.0f);

			return true;
		}));
}


//============================================================================
// 斬首
//============================================================================
void CSceneSelect::DownCutter()
{
	// 全てのカッターを落とす
	for (const auto& rIt : m_vpCutter)
	{
		if (CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(rIt->GetCollider()))
		{
			// 特定の高さまで下げる
			OBJ::Transform CutterTransform = pRigidBody->GetWorldTransform();
			useful::ExponentialDecay(CutterTransform.Pos.y, 1.675f, 0.2f);
			pRigidBody->SetWorldTransform(CutterTransform);
		}
	}
}

//============================================================================
// 整列
//============================================================================
void CSceneSelect::Alignment()
{
	unsigned char wNumHead = static_cast<unsigned char>(m_vpPM.size());

	// 全てのヘッドが停止したら
	for (unsigned char wIdx = 0; wIdx < wNumHead; ++wIdx)
	{
		CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(m_vpPM[wIdx]->GetCollider());

		// ① プレイヤー用の整列位置
		DirectX::XMFLOAT3 InitPos = { useful::VEC3_ZERO_INIT };
		const float f = -3.75f + (wIdx * 2.5f);
		InitPos = { f, 5.75f, 0.5f };

		// ② プレイヤーの整列トランスフォームを設定
		OBJ::Transform PlayersInitTransform =
		{
			{ 0.5f, 0.5f, 0.5f },
			{ 0.0f, 0.0f, 0.0f, 1.0f},
			InitPos
		};

		// ③ 現在のワールドトランスフォームを取得
		OBJ::Transform CurrentTransform = pRigidBody->GetWorldTransform();

		// ④ 位置を補間
		useful::ExponentialDecay(CurrentTransform.Pos.x, PlayersInitTransform.Pos.x, 0.05f);
		useful::ExponentialDecay(CurrentTransform.Pos.y, PlayersInitTransform.Pos.y, 0.05f);
		useful::ExponentialDecay(CurrentTransform.Pos.z, PlayersInitTransform.Pos.z, 0.05f);

		// ⑤ 向きを補間
		useful::ExponentialDecay(CurrentTransform.Rot.x, PlayersInitTransform.Rot.x, 0.05f);
		useful::ExponentialDecay(CurrentTransform.Rot.y, PlayersInitTransform.Rot.y, 0.05f);
		useful::ExponentialDecay(CurrentTransform.Rot.z, PlayersInitTransform.Rot.z, 0.05f);

		// ⑥ ワールドトランスフォームを設定
		pRigidBody->SetActive();
		pRigidBody->SetWorldTransform(CurrentTransform);
	}

	// カメラの初期設定
	CCamera* pCamera = CRenderer::RefInstance().GetCamera();
	pCamera->SetPosTarget({ 0.0f, 7.0f, 0.5f });
}

//============================================================================
// シンボルの生成
//============================================================================
void CSceneSelect::SpawnSymbol()
{
	// ヘッドの数を取得
	unsigned char wNumHead = static_cast<unsigned char>(m_vpPM.size());

	// 全てのヘッドにシンボルを生成
	for (unsigned char wIdx = 0; wIdx < wNumHead; ++wIdx)
	{
		if (m_apSymbol[wIdx])
		{
			continue;
		}

		// シンボルの生成
		CSymbol* pSymbol = CObjectManager::CreateRaw<CSymbol>();

	   	// テクスチャの設定
		pSymbol->SetSymbolIdx(wIdx);

		// 保有
		m_apSymbol[wIdx] = pSymbol;
	}
}

//============================================================================
// シンボルセット
//============================================================================
void CSceneSelect::SetSymbol()
{
	for (unsigned char wIdx = 0; wIdx < MAX_PLAYER; ++wIdx)
	{
		if (!m_apSymbol[wIdx])
		{
			continue;
		}

		// シンボルのトランスフォームの取得
		OBJ::Transform SymbolTransform = m_apSymbol[wIdx]->GetTransform();

		// シンボルの位置をヘッドの位置に合わせる
		SymbolTransform.Size   = { 1.0f, 1.0f, 1.0f };
		SymbolTransform.Pos    = m_vpPM[wIdx]->GetTransform().Pos;
		SymbolTransform.Pos.y += 1.25f;

		// シンボルのトランスフォームを設定
		m_apSymbol[wIdx]->SetTransform(SymbolTransform);
	}
}

//============================================================================
// ステージ選択HUDの生成
//============================================================================
void CSceneSelect::SpawnStageHud()
{
	// ヘッドの数を取得
	unsigned char wNumHead = static_cast<unsigned char>(m_vpPM.size());

	for (unsigned char wIdx = 0; wIdx < wNumHead; ++wIdx)
	{
		if (m_apStageHud[wIdx])
		{
			continue;
		}

		// ステージHUDの生成
		CRect3D* pStageHud = CObjectManager::CreateRaw<CRect3D>();

		// テクスチャの設定
		pStageHud->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Human.D"));

		// 保有
		m_apStageHud[wIdx] = pStageHud;
	}
}

//============================================================================
// ステージ選択HUDのセット
//============================================================================
void CSceneSelect::SetStageHud()
{
	for (unsigned char wIdx = 0; wIdx < MAX_PLAYER; ++wIdx)
	{
		if (!m_apStageHud[wIdx])
		{
			continue;
		}

		// ステージHUDのトランスフォームの取得
		OBJ::Transform HudTransform = m_apStageHud[wIdx]->GetTransform();

		// ステージHUDの位置をヘッドの位置に合わせる
		HudTransform.Size = { 2.0f, 2.0f, 2.0f };
		HudTransform.Rot = { DirectX::XM_PI, 0.0f, 0.0f, 1.0f };
		HudTransform.Pos = m_vpPM[wIdx]->GetTransform().Pos;
		HudTransform.Pos.y += 3.0f;

		// ステージインデックスに応じたテクスチャを設定
		switch (m_nStageIdx[wIdx])
		{
		case 0:
			m_apStageHud[wIdx]->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Human.D"));
			break;

		case 1:
			m_apStageHud[wIdx]->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Sky"));
			break;

		case 2:
			m_apStageHud[wIdx]->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Test"));
			break;

		default:
			break;
		}

		// もう決定されていたらカラーを薄くする
		if (m_nStageDecide[wIdx])
		{
			m_apStageHud[wIdx]->SetCol({ 1.0f, 1.0f, 1.0f, 0.5f });
		}
		else
		{
			m_apStageHud[wIdx]->SetCol({ 1.0f, 1.0f, 1.0f, 1.0f });
		}

		// ステージHUDのトランスフォームを設定
		m_apStageHud[wIdx]->SetTransform(HudTransform);
	}
}

//============================================================================
// ステージ選択
//============================================================================
void CSceneSelect::SelectStage()
{
	// インプットマネージャーの参照
	CInputManager& CInputManager = CInputManager::RefInstance();

	// ヘッドの生成数を取得
	unsigned char wNumHead = static_cast<unsigned char>(m_vpPM.size());

	// 誰かが選択していなければ立てるフラグ
	bool bAnyoneHasNot = false;

	// 人数分のコントローラーをチェック
	for (unsigned char wIdx = 0; wIdx < wNumHead; ++wIdx)
	{
		// まだステージ決定していなかったら
		if (!m_nStageDecide[wIdx])
		{
			if (CInputManager.GetTrackerGamePad(wIdx).a == DirectX::GamePad::ButtonStateTracker::PRESSED)
			{
				// ステージ決定フラグを立てる
				m_nStageDecide[wIdx] = true;

				// 効果音：スタート
				CSoundManger::RefInstance().Play("CntDown", false, -0.5f, 2.0f);
			}
		   	else if (CInputManager.GetTrackerGamePad(wIdx).leftStickLeft == DirectX::GamePad::ButtonStateTracker::PRESSED)
			{
				m_nStageIdx[wIdx] > 0 ? --m_nStageIdx[wIdx] : m_nStageIdx[wIdx] = 1;

				// 効果音：ジャンプ
				CSoundManger::RefInstance().Play("Jump", false, -0.5f, 1.0f);
			}
			else if (CInputManager.GetTrackerGamePad(wIdx).leftStickRight == DirectX::GamePad::ButtonStateTracker::PRESSED)
			{
				m_nStageIdx[wIdx] < 1 ? ++m_nStageIdx[wIdx] : m_nStageIdx[wIdx] = 0;
			
				// 効果音：ジャンプ
				CSoundManger::RefInstance().Play("Jump", false, -0.5f, 1.0f);
			}

			bAnyoneHasNot = true;
		}
	}

	// 全員が決定したら通知のフラグを立てる
	if (!bAnyoneHasNot)
	{
		m_bStageDecideAll = true;

		// ストップカウンターをリセット
		// これはステージ抽選処理に使う
		m_nStopCnt = 0;
	}
}

//============================================================================
// ステージ決定
//============================================================================
void CSceneSelect::DecideStage()
{
	// ストップカウンターが一定値に達したら確定
	if (m_nStopCnt > 180 + rand() % 60)
	{
		m_bStageDecided = true;

		if (m_pHud_CB)
		{
			m_pHud_CB->SetDeath();
		}
	}

	++m_nCntChangeStage;
	
	// ステージ変更カウントが溜まっていなければ処理しない
	if (m_nCntChangeStage < 5)
	{
		return;
	}

	// 決定しているステージHUDの数を数える
	int nStageHudCount = 0;
	for (unsigned char wIdx = 0; wIdx < MAX_PLAYER; ++wIdx)
	{
		if (!m_apStageHud[wIdx])
		{
			continue;
		}

		++nStageHudCount;
	}

	// 毎フレーム抽選対象のインデックスを進める
	m_nRandomIdx < nStageHudCount - 1 ? ++m_nRandomIdx : m_nRandomIdx = 0;

	// 抽選対象のHUDに色を付ける
	for (unsigned char wIdx = 0; wIdx < MAX_PLAYER; ++wIdx)
	{
		if (!m_apStageHud[wIdx])
		{
			continue;
		}

		if (wIdx == m_nRandomIdx)
		{
			m_apStageHud[wIdx]->SetCol({ 1.0f, 1.0f, 1.0f, 1.0f });
		}
		else
		{
			m_apStageHud[wIdx]->SetCol({ 1.0f, 1.0f, 1.0f, 0.5f });
		}
	}

	// ステージ変更カウントをリセット
	m_nCntChangeStage = 0;

	// 効果音
	CSoundManger::RefInstance().Play("CntDown", false, -0.5f, 1.0f);
}

//============================================================================
// ステージ確定アピール
//============================================================================
void CSceneSelect::DecideAppeal()
{
	for (unsigned char wIdx = 0; wIdx < MAX_PLAYER; ++wIdx)
	{
		if (!m_apStageHud[wIdx])
		{
			continue;
		}

		// トランスフォームを取得
		OBJ::Transform HudTransform = m_apStageHud[wIdx]->GetTransform();

		if (wIdx == m_nRandomIdx)
		{
			// カメラの位置を取得
			CCamera* pCamera = CRenderer::RefInstance().GetCamera();
			const DirectX::XMFLOAT3 CameraPos = pCamera->GetPos();

			// カメラの位置に合わせて補間
			useful::ExponentialDecay(HudTransform.Size.x, 12.5f, 0.025f);
			useful::ExponentialDecay(HudTransform.Size.y, 12.5f, 0.025f);
			useful::ExponentialDecay(HudTransform.Pos.x, CameraPos.x, 0.1f);
			useful::ExponentialDecay(HudTransform.Pos.y, CameraPos.y, 0.1f);
			useful::ExponentialDecay(HudTransform.Pos.z, CameraPos.z, 0.1f);

			if (HudTransform.Size.x > 11.0f)
			{
				m_bChangeScene = true;
			}
		}
		else
		{
			// 消滅
			useful::ExponentialDecay(HudTransform.Size.x, 0.0f, 0.2f);
			useful::ExponentialDecay(HudTransform.Size.y, 0.0f, 0.2f);
		}

		// トランスフォームを設定
		m_apStageHud[wIdx]->SetTransform(HudTransform);
	}
}