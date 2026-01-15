//============================================================================
// 
// リザルトシーン実装ファイル [scene.result.cpp]
// Author : Copilot
// 
//============================================================================

#include "scene.result.h"
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

#include "scene.title.h"
#include <player.h>

//============================================================================
// コンストラクタ
//============================================================================
CSceneResult::CSceneResult(const std::vector<float>& playerSurvivalTimes)
    : m_pBackground(nullptr), m_nResultValue(0), m_fGameTime(0.0f),
    m_playerSurvivalTimes(playerSurvivalTimes)
{
    const int SCREEN_W = 1920;
    const int SCREEN_H = 1080;

    const float CENTER_X = SCREEN_W / 2.0f;  // 960.0f
    const float CENTER_Y = SCREEN_H / 2.0f;  // 540.0f

    auto pBG = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::BG);
    pBG->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("BG")); // 必要に合わせて
    OBJ::Transform TF_BG = { {SCREEN_W, SCREEN_H, 0.0f}, {0,0,0,0}, {CENTER_X, CENTER_Y, 0.0f} };
    pBG->SetTransform(TF_BG);
    pBG->SetTransformTarget(TF_BG);
    m_pBackground = pBG;

    const float BASE_Y = 880.0f;      // タイマー表示高さ
    const float TIMER_INTERVAL = 360.0f; // タイマー間隔（128px分のHUD＋余白で余裕）

    size_t playerCount = m_playerSurvivalTimes.size();
    float totalWidth = (playerCount - 1) * TIMER_INTERVAL;
    float leftmostX = CENTER_X - totalWidth / 2.0f; // 一番左のタイマーX座標

    for (size_t i = 0; i < playerCount; ++i)
    {
        std::vector<CHud*> vpNums;
        float surv = m_playerSurvivalTimes[i];
        int totalSec = static_cast<int>(surv);
        int minutes = totalSec / 60;
        int seconds = totalSec % 60;

        float baseX = leftmostX + i * TIMER_INTERVAL;

        // 分
        auto pMin = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pMin->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNum" + std::to_string(minutes)));
        pMin->SetTransform({ {32, 48, 0}, {0,0,0,0}, {baseX, BASE_Y, 0} });
        pMin->SetTransformTarget(pMin->GetTransform());
        vpNums.push_back(pMin);

        // コロン
        auto pColon = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pColon->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNumCoron"));
        pColon->SetTransform({ {20, 48, 0}, {0,0,0,0}, {baseX + 36, BASE_Y, 0} });
        pColon->SetTransformTarget(pColon->GetTransform());
        vpNums.push_back(pColon);

        // 秒（十の位）
        auto pSec10 = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pSec10->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNum" + std::to_string(seconds / 10)));
        pSec10->SetTransform({ {32, 48, 0}, {0,0,0,0}, {baseX + 60, BASE_Y, 0} });
        pSec10->SetTransformTarget(pSec10->GetTransform());
        vpNums.push_back(pSec10);

        // 秒（一の位）
        auto pSec1 = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pSec1->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNum" + std::to_string(seconds % 10)));
        pSec1->SetTransform({ {32, 48, 0}, {0,0,0,0}, {baseX + 92, BASE_Y, 0} });
        pSec1->SetTransformTarget(pSec1->GetTransform());
        vpNums.push_back(pSec1);

        m_vvPlayerNumbers.push_back(vpNums);
        // HUD描画はDraw()でループ
    }
}

//============================================================================
// デストラクタ
//============================================================================
CSceneResult::~CSceneResult()
{
	// CHudはObjectManagerがデストラクションを管理する前提
}

//============================================================================
// 更新
//============================================================================
void CSceneResult::Update()
{
	if (CInputManager::RefInstance().EnhancedEnter()) 
	{
        CPlayer::ClearAllSurvivalTimes();
		Change();
	}
}

void CSceneResult::Change()
{
	// 全オブジェクトに死亡フラグを立てる
	CObjectManager::RefInstance().SetDeathAll();

	CSceneManager::RefInstance().ChangeScene(std::make_unique<CSceneTitle>());
}