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

	enum STAGE
	{
		FIRST = 0,
		SECOND,
		THARD,
		MAX,
	};
	
	int NowStage = FIRST;
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSceneSelect::CSceneSelect()
	: m_nCommonCnt(0)
	, m_vpBeamLight{}
	, m_vBeamLightQue{}
	, m_pCursor(nullptr)
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

	// カーソルの生成
	auto pCursor = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
	pCursor->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Cursor"));
	m_pCursor = pCursor;
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
	/* デバッグ */
	AAA();

	// ビームライトの生成キュー
	WhileEvent_QueInstantiateLight();

	// ステージの変更
	StageChange();

	// カーソルの追従
	WhileEvent_CursorTrack();

	if (CInputManager::RefInstance().EnhancedEnter())
	{
		Change();
	}
}

//============================================================================
// シーン変更
//============================================================================
void CSceneSelect::Change()
{
	// 全オブジェクトに死亡フラグを立てる
	CObjectManager::RefInstance().SetDeathAll();

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
		//CSoundManger::RefInstance().Play("Light", false, -0.5f, 2.0f);
	}
}

//============================================================================
// カーソルの追従
//============================================================================
void CSceneSelect::WhileEvent_CursorTrack()
{
	//// カーソルのスパンを設定
	//const float fSpan = 32.0f;
	///* 消去 */
	//static float fRotZ = 0.0f;
	//static float fTick = 0.0f;
	///* 向き：Z軸：ゆらゆら */
	//++fTick;
	//fRotZ = sinf(fTick * 0.5f);
	//// 移動速度
	//float speed = 200.0f;
	//float MoveDirection = 0.0f;
	//// 接続数取得
	//int ConnectedGamePadNum = CInputManager::RefInstance().GetConnectedGamePadNum();
	//for (int i = 0; i < ConnectedGamePadNum; i++)
	//{
	//	// 入力方向を取得
	//	const std::optional<float>& InputToMoveDirection = CInputManager::RefInstance().ConvertInputToMoveDirection(i);
	//	// 入力されてる
	//	if (InputToMoveDirection)
	//	{
	//		// 数値を取得
	//		MoveDirection = InputToMoveDirection.value();
	//		//元の位置を取得
	//		DirectX::XMFLOAT3 Pos = m_pCursor->GetTransform().Pos;
	//		// 新しい位置を計算
	//		DirectX::XMFLOAT3 NewPos =
	//		{
	//			Pos.x + sinf(MoveDirection) * speed,
	//			Pos.y - cosf(MoveDirection) * speed,
	//			0.0f
	//		};
	//		// カーソル位置を元にHUDのトランスフォームを作成
	//		const OBJ::Transform NewTF = {
	//			{ fSpan, fSpan, 0.0f },
	//			{ 0.0f, 0.0f, -1.57f, 0.0f },
	//			 NewPos
	//		};
	//		// 目標トランスフォームを上書き
	//		m_pCursor->SetTransformTarget(NewTF);
	//	}
	//}


	const float WCX = OBJ::CalcCenterOfWindow().x;	// 画面の中心
	float fW = 1980.0f / 1280.0f;
	float Posx = WCX * 0.5f;						// 半分の位置

	OBJ::Transform TF = m_pCursor->GetTransform();

	TF = {
		{ 32.0f,32.0f,0.0f },
		{ 0.0f, 0.0f, -1.57f,0.0f },
		{ (Posx + (Posx * NowStage)) * fW,100.0f,0.0f },
	};
	m_pCursor->SetTransform(TF);
	//m_pCursor->SetTransformTarget(TF);
}

//============================================================================
// ステージ変更
//============================================================================
void CSceneSelect::StageChange()
{
	if (CInputManager::RefInstance().GetTrackerGamePad(0).leftStickLeft == DirectX::GamePad::ButtonStateTracker::PRESSED)
	{
		int Num = static_cast<int>(NowStage) - 1;

		if (Num < 0)
		{
			Num = static_cast<int>(THARD);
		}

		NowStage = static_cast<STAGE>(Num);

	}
	if (CInputManager::RefInstance().GetTrackerGamePad(0).leftStickRight == DirectX::GamePad::ButtonStateTracker::PRESSED)
	{
		NowStage = static_cast<STAGE>(static_cast<int>(NowStage) + 1);

		if (NowStage == MAX)
		{
			NowStage = FIRST;
		}
	}
}