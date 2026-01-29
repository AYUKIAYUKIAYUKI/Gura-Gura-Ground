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
#include "API.object.manager.h"
#include "API.texture.manager.h"
#include "API.hud.h"
#include "API.fullscreen.2D.h"
#include "beamlight.h"

#include "API.gltf.manager.h"
#include "API.physics.model.h"
#include "API.rigidbody.h"
#include "player.fake.h"

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
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSceneSelect::CSceneSelect()
	: m_nCommonCnt(0)
	, m_vpBeamLight{}
	, m_vBeamLightQue{}
	, m_bDeathPenaly(false)
	, m_bSelectStart(false)
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
	if (!m_bDeathPenaly)
	{
		// ビームライトの生成キュー
		WhileEvent_QueInstantiateLight();

		// 接続チェック
		WhileEvent_CennectCheck();

		// 死刑執行
		if (CInputManager::RefInstance().EnhancedEnter())
		{
			m_bDeathPenaly = true;

			// ヘッドを転がす
			for (const auto& rIt : m_vpPM)
			{
				CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(rIt->GetCollider());

				// ダイナミック化
				pRigidBody->SetDynamic();
			}

			// ボディを転がす
			for (const auto& rIt : m_vpBody)
			{
				CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(rIt->GetCollider());

				// ダイナミック化
				pRigidBody->SetDynamic();
			}
		}
	}
	else if (m_bSelectStart)
	{
#if 0
		// 整列
		Alignment();

		if (CInputManager::RefInstance().EnhancedEnter())
		{
			// 全オブジェクトに死亡フラグを立てる
			CObjectManager::RefInstance().SetDeathAll();

			//タイトルBGMを停止する
			CSoundManger::RefInstance().Stop("BGM_TITLE");

			// ゲームシーンへ
			CSceneManager::RefInstance().ChangeScene(std::make_unique<CSceneSelect>());
		}
#else
		Change();
#endif
	}
	else
	{
		// 斬首
		DownCutter();
	}
}

//============================================================================
// シーン変更
//============================================================================
void CSceneSelect::Change()
{
	// 全オブジェクトに死亡フラグを立てる
	CObjectManager::RefInstance().SetDeathAll();

	//タイトルBGMを停止する
	CSoundManger::RefInstance().Stop("BGM_TITLE");

	// ゲームシーンへ
	CSceneManager::RefInstance().ChangeScene(std::make_unique<CSceneGame>());
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
	unsigned char wNumPad = CInputManager::RefInstance().GetConnectedGamePadNum() + 3;
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
		m_vpPM[wSize]->SetDeath();
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
	if (wIdx > 1)
	{
		CutterInitPos.x += -0.025f * wIdx;
	}
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
		CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(rIt->GetCollider());

		// 特定の高さまで下げる
		OBJ::Transform CutterTransform = pRigidBody->GetWorldTransform();
		useful::ExponentialDecay(CutterTransform.Pos.y, 1.675f, 0.2f);
		pRigidBody->SetWorldTransform(CutterTransform);

#ifdef _DEBUG
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		ImGui::Text("Cutter Y : %.2f", CutterTransform.Pos.y);
		ImGui::End();
#endif // _DEBUG
	}

	// 全てのヘッドが停止したら
	for (const auto& rIt : m_vpPM)
	{
		CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(rIt->GetCollider());

		if (pRigidBody->GetActive())
		{
			break;
		}

		m_bSelectStart = true;
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

		// プレイヤー用の整列位置
		DirectX::XMFLOAT3 InitPos = { useful::VEC3_ZERO_INIT };
		const float f = -3.75f + (wIdx * 2.5f);
		InitPos = { f, 0.0f, -5.0f };

		// プレイヤーの整列トランスフォームを設定
		OBJ::Transform PlayersInitTransform =
		{
			{ 0.5f, 0.5f, 0.5f },
			{ 0.0f, 0.0f, 0.0f, 1.0f},
			InitPos
		};

		pRigidBody->SetActive();
		pRigidBody->SetWorldTransform(PlayersInitTransform);
	}

	// カメラの初期設定
	CCamera* pCamera = CRenderer::RefInstance().GetCamera();
	pCamera->SetPosTarget({ 0.0f, 0.0f, -5.0f });
}