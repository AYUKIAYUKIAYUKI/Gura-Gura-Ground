//============================================================================
// 
// タイトルシーン [scene.title.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "scene.title.h"
#include "API.sound.manager.h"

// 遷移先のシーン
#include "API.scene.manager.h"
#include "scene.select.h"

// インプット取得のため
#include "API.input.manager.h"

// オブジェクト生成・破棄のため
#include "API.renderer.h"
#include "API.object.manager.h"
#include "API.texture.manager.h"
#include "API.hud.h"
#include "API.fullscreen.2D.h"

/* デバッグ */
namespace
{
	static float fA = 250.0f;
	static float fB = 344.0f;
	static float fC = 640.0f;
	static float fD = 360.0f;

	static CHud* Ijiru = nullptr;

	void AAA()
	{
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");

		ImGui::DragFloat("A : ", &fA, 0.1f);
		ImGui::DragFloat("B : ", &fB, 0.1f);
		ImGui::DragFloat("C : ", &fC, 0.1f);
		ImGui::DragFloat("D : ", &fD, 0.1f);

		if (Ijiru && CInputManager::RefInstance().GetStateKeyboard().Space)
		{
			OBJ::Transform TF = { { fA, fB, 0.0f }, { useful::VEC4_ZERO_INIT }, { fC, fD, 0.0f } };
			Ijiru->SetTransformTarget(TF);
		}

		ImGui::Separator();
		ImGui::End();
	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSceneTitle::CSceneTitle()
	: m_nCommonCnt(0)
	, m_pBackCurtain(nullptr)
	, m_vpSatinCurtain{}
	, m_vpLogo{}
	, m_bAnimationStateLogo(false)
	, m_bTransition(false)
{
	// 定数：初期トランスフォーム全ゼロクリア
	const float WCX           = OBJ::CalcCenterOfWindow().x;
	const float WCY           = OBJ::CalcCenterOfWindow().y;
	const OBJ::Transform ITFA = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -15.0f, 0.0f }, { WCX, 0.0f, 0.0f } };
	const OBJ::Transform ITFB = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,  15.0f, 0.0f }, { WCX,  WCY, 0.0f } };

	// 幕を生成
	auto pBG = CObject::Create<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::BG);
	pBG->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("BG"));
	OBJ::Transform TF = { { 1280.0f, 1280.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { WCX, WCY, 0.0f } };
	pBG->SetTransform(TF);
	pBG->SetTransformTarget(TF);
	m_pBackCurtain = pBG;

	// 額縁を生成
	auto pFrame = CObject::Create<CHud>();
	pFrame->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Frame"));
	TF = { { 500.0f, 500.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { WCX, WCY, 0.0f } };
	pFrame->SetTransform(TF);
	pFrame->SetTransformTarget(TF);

	// 無を生成
	auto pPicture = CObject::Create<CHud>();
	pPicture->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Human.D"));
	TF = { { fA, fB, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { WCX, WCY, 0.0f } };
	pPicture->SetTransform(TF);
	pPicture->SetTransformTarget(TF);
	Ijiru = pPicture;

	// 緞帳(左)を生成
	auto pSatinCurtain = CObject::Create<CHud>();
	pSatinCurtain->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Human.D"));
	TF = { { 600.0f, 720.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 300.0f, WCY, 0.0f } };
	pSatinCurtain->SetTransform(TF);
	pSatinCurtain->SetTransformTarget(TF);
	m_vpSatinCurtain.push_back(pSatinCurtain);

	// 緞帳(右)を生成
	pSatinCurtain = CObject::Create<CHud>();
	pSatinCurtain->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Human.D"));
	TF = { { 600.0f, 720.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 1280.0 - 300.0f, WCY, 0.0f } };
	pSatinCurtain->SetTransform(TF);
	pSatinCurtain->SetTransformTarget(TF);
	m_vpSatinCurtain.push_back(pSatinCurtain);

	// 地面を作成
	auto Logo = CObject::Create<CHud>();
	Logo->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Logo.A"));
	Logo->SetTransform(ITFB);
	TF = { { 624.0f, 113.0f, 0.0f }, { 0.0f, 0.0f, 0.1f, 0.0f }, { WCX + 125.0f, WCY + 125.0f, 0.0f } };
	Logo->SetTransformTarget(TF);
	m_vpLogo.push_back(Logo);

	//「ぐらぐら」を生成
	Logo = CObject::Create<CHud>();
	Logo->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Logo.B"));
	Logo->SetTransform(ITFA);
	TF = { { 417.0f, 134.0f, 0.0f }, { 0.0f, 0.0f, -0.2f, 0.0f }, { WCX - 200.0f, WCY - 125.0f, 0.0f } };
	Logo->SetTransformTarget(TF);
	m_vpLogo.push_back(Logo);

	//「ぐらうんど」を生成
	Logo = CObject::Create<CHud>();
	Logo->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Logo.C"));
	Logo->SetTransform(ITFB);
	TF = { { 529.0f, 135.0f, 0.0f }, { 0.0f, 0.0f, 0.1f, 0.0f }, { WCX + 125.0f, WCY + 40.0f, 0.0f } };
	Logo->SetTransformTarget(TF);
	m_vpLogo.push_back(Logo);
}

//============================================================================
// デストラクタ
//============================================================================
CSceneTitle::~CSceneTitle()
{}

//============================================================================
// 更新処理
//============================================================================
void CSceneTitle::Update()
{
	AAA();

	// 遷移状態に入っていたら
	if (m_bTransition)
	{
		++m_nCommonCnt;

		/* 条件を変更します */
		if (m_nCommonCnt > 120)
		{
			// セレクトシーンへ変更
			CSceneManager::RefInstance().ChangeScene(std::make_unique<CSceneSelect>());
		}

		return;
	}

	// ロゴ動作
	if (m_bAnimationStateLogo)
	{
		WhileEvent_AnimationLogo();
	}
	else
	{
		++m_nCommonCnt;

		// 初回のみ、一定時間経過でロゴのアニメーション状態をセット
		if (m_nCommonCnt > 120)
		{
			m_nCommonCnt          = 0;
			m_bAnimationStateLogo = true;
		}
	}

	// シーン変更
	if (CInputManager::RefInstance().EnhancedEnter())
	{
		Change();
	}
}

//============================================================================
// シーン変更
//============================================================================
void CSceneTitle::Change()
{
	// かんたんなトグルで
	m_bTransition = true;

	// 遷移カウント計測のためリセット
	m_nCommonCnt = 0;

	// 緞帳移動
	TriggerEvent_MoveSatinCurtain();

	// ロゴ消去
	TriggerEvent_DisappearLogo();

	// 幕を暗く
	TriggerEvent_DarkFadeAllCurtain();

	// 効果音：ブザー
	CSoundManger::RefInstance().Play("Buzzer", false, -0.5f, 1.5f);

	// 効果音：ジャンプ
	CSoundManger::RefInstance().Play("Jump", false, 0.0f, 1.0f);
}

//============================================================================
// 緞帳移動：それぞれが画面端の位置へ…X軸のみ平行移動
//============================================================================
void CSceneTitle::TriggerEvent_MoveSatinCurtain()
{
	// X方向の移動目標
	const float fPosTarget_X = -100.0f;

	// 緞帳(左)の目標位置を変更
	const OBJ::Transform& rTF_Left   = m_vpSatinCurtain[0]->GetTransformTarget();
	const OBJ::Transform  NewTF_Left = { rTF_Left.Size, rTF_Left.Rot, { fPosTarget_X, rTF_Left.Pos.y, 0.0f } };
	m_vpSatinCurtain[0]->SetTransformTarget(NewTF_Left);

	// 緞帳(右)の目標位置を変更
	const OBJ::Transform& rTF_Right   = m_vpSatinCurtain[1]->GetTransformTarget();
	const OBJ::Transform  NewTF_Right = { rTF_Right.Size, rTF_Right.Rot, { 1280.0f + -fPosTarget_X, rTF_Right.Pos.y, 0.0f } };
	m_vpSatinCurtain[1]->SetTransformTarget(NewTF_Right);
}

//============================================================================
// 幕を暗く
//============================================================================
void CSceneTitle::TriggerEvent_DarkFadeAllCurtain()
{
	// 暗さ
	const float fDarkness = 0.25f;

	// 暗色作成
	const DirectX::XMFLOAT4 NewCol = { fDarkness, fDarkness, fDarkness, 1.0f };

	// カラー反映：奥の幕
	m_pBackCurtain->SetColTarget(NewCol);

	// カラー反映：緞帳
	for (const auto& rIt : m_vpSatinCurtain)
	{
		rIt->SetColTarget(NewCol);
	}
}

//============================================================================
// ロゴ消去：全てのロゴがその場でフェードアウトする
//============================================================================
void CSceneTitle::TriggerEvent_DisappearLogo()
{
	// 表示されているHUDをはける
	for (const auto& rIt : m_vpLogo)
	{
		const OBJ::Transform& rTF   = rIt->GetTransformTarget();
		const OBJ::Transform  NewTF = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -15.0f, 0.0f }, rTF.Pos };
		rIt->SetTransformTarget(NewTF);
	}
}

//============================================================================
// ロゴ動作
//============================================================================
void CSceneTitle::WhileEvent_AnimationLogo()
{
	++m_nCommonCnt;

	// 一定時間の経過で…
	if (m_nCommonCnt > 60)
	{
		m_nCommonCnt = 0;

		// 保有しているHUDのオブジェクト全て、現在サイズのみ強制拡大しパウンド
		for (const auto& rIt : m_vpLogo)
		{
			const OBJ::Transform& rTF   = rIt->GetTransformTarget();
			const OBJ::Transform  NewTF = { { rTF.Size.x + -10.0f, rTF.Size.y + 10.0f, 0.0f }, rTF.Rot, rTF.Pos };
			rIt->SetTransform(NewTF);
		}
	}
}