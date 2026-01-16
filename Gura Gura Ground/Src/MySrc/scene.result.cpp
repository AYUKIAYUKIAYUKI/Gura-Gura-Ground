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
    pBG->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("BG"));
    OBJ::Transform TF_BG = { {SCREEN_W, SCREEN_H, 0.0f}, {0,0,0,0}, {CENTER_X, CENTER_Y, 0.0f} };
    pBG->SetTransform(TF_BG);
    pBG->SetTransformTarget(TF_BG);
    m_pBackground = pBG;
    const float BASE_Y = 980.0f;

    const int TIMER_ADJUST = 50;
    const float TIMER_INTERVAL = 455.0f;
    const float ICON_Y_OFFSET = -160.0f;
    const float ICON_SIZE_W = 77.0f;
    const float ICON_SIZE_H = 69.0f;
    const float RANK_Y_OFFSET = 40.0f;    // ランク画像Y位置補正

    size_t playerCount = m_playerSurvivalTimes.size();
    float totalWidth = (playerCount - 1) * TIMER_INTERVAL;
    float leftmostX = CENTER_X - totalWidth / 2.0f;

    struct PlayerRankInfo {
        int idx;
        int timeInt;
        float timeOrig;
    };
    std::vector<PlayerRankInfo> sortList;
    for (size_t i = 0; i < playerCount; ++i) 
    {
        sortList.push_back(PlayerRankInfo{ (int)i, (int)m_playerSurvivalTimes[i], m_playerSurvivalTimes[i] });
    }

    // タイム大きい順（降順）でsort
    std::sort(sortList.begin(), sortList.end(),
        [](const PlayerRankInfo& a, const PlayerRankInfo& b) {
            return a.timeInt > b.timeInt || (a.timeInt == b.timeInt && a.idx < b.idx);
        }
    );
    std::vector<int> indexToRank(playerCount, 0); // index→順位（Rank_番号）
    int rankNumber = 1; // 1位（Rank_1）から
    int prevTime = -1;
    int usedRank = rankNumber;
    for (size_t sortedPos = 0; sortedPos < sortList.size(); ++sortedPos)
    {
        const auto& info = sortList[sortedPos];
        if (sortedPos == 0 || info.timeInt != prevTime) {
            usedRank = rankNumber;
        }
        indexToRank[info.idx] = usedRank;
        prevTime = info.timeInt;
        rankNumber++;
    }

    // IconPlayer00X配置
    auto pWinText = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
    pWinText->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("WinText"));
    OBJ::Transform WIN_TEXT_TR = { {275, 146, 0}, {0,0,0,0}, {1500.0f, 300.0f, 0} };
    pWinText->SetTransform(WIN_TEXT_TR);
    pWinText->SetTransformTarget(WIN_TEXT_TR);
    m_vpPlayerIcons.push_back(pWinText);

    int maxTimeInt = -1;
    for (size_t i = 0; i < playerCount; ++i) {
        if ((int)m_playerSurvivalTimes[i] > maxTimeInt)
            maxTimeInt = (int)m_playerSurvivalTimes[i];
    }

    // 最長タイムプレイヤーにTextPlayer00X画像を表示
    for (size_t i = 0; i < playerCount; ++i) {
        if ((int)m_playerSurvivalTimes[i] == maxTimeInt) {
            float baseX = leftmostX + i * TIMER_INTERVAL;
            std::string textPlayerTex = "TextPlayer00" + std::to_string(i + 1); // 例: TextPlayer001.png
            auto pText = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
            pText->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey(textPlayerTex));
            // 画像のサイズを参考に座標調整（幅:664px, 高さ:133px想定）
            OBJ::Transform TEXT_PLAYER_TR = { {664, 133, 0}, {0,0,0,0}, {1500, 500.0f, 0} };
            pText->SetTransform(TEXT_PLAYER_TR);
            pText->SetTransformTarget(TEXT_PLAYER_TR);
            m_vpPlayerTextImgs.push_back(pText);
        }
    }

    for (size_t i = 0; i < playerCount; ++i)
    {
        std::vector<CHud*> vpNums;
        float surv = m_playerSurvivalTimes[i];
        int totalSec = static_cast<int>(surv);
        int minutes = totalSec / 60;
        int seconds = totalSec % 60;

        float baseX = leftmostX + i * TIMER_INTERVAL;

        // IconPlayer00X配置
        std::string iconTex = "IconPlayer00" + std::to_string(i + 1);
        auto pWinText = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pWinText->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey(iconTex));
        float iconX = baseX - 220 + TIMER_ADJUST;
        OBJ::Transform ICON_TR = { {ICON_SIZE_W, ICON_SIZE_H, 0}, {0,0,0,0}, {iconX, 800.0f, 0} };
        pWinText->SetTransform(ICON_TR);
        pWinText->SetTransformTarget(ICON_TR);
        m_vpPlayerIcons.push_back(pWinText);

        // PlayerLight画像配置
        std::string playerLightTex = "PlayerLight00" + std::to_string(i + 1);
        auto pLight = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pLight->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey(playerLightTex));
        OBJ::Transform TF_Light = { {705, 610, 0}, {0,0,0,0}, {baseX + 10.0f, 910.0f, 0} };
        pLight->SetTransform(TF_Light);
        pLight->SetTransformTarget(TF_Light);
        m_vpPlayerLights.push_back(pLight);

        //ランク画像の配置
        int rankImageNumber = indexToRank[i];
        std::string rankTex = "Rank_" + std::to_string(rankImageNumber);
        auto pRank = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        OBJ::Transform RANK_TR = { {134, 94, 0}, {0,0,0,0}, {baseX + 100, BASE_Y - 100.0f, 0} };
        pRank->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey(rankTex));
        pRank->SetTransform(RANK_TR);
        pRank->SetTransformTarget(RANK_TR);
        m_vpPlayerRankImgs.push_back(pRank);

        // プレイヤー個別のRankText配置
        auto pIndivRANKTEXT = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        OBJ::Transform INDIV_RANK_TEXT = { {107, 59, 0.0f}, {0,0,0,0}, {baseX - 210 + TIMER_ADJUST, BASE_Y - 90.0f, 0.0f} };
        pIndivRANKTEXT->SetTransform(INDIV_RANK_TEXT);
        pIndivRANKTEXT->SetTransformTarget(INDIV_RANK_TEXT);
        m_vpPlayerBattleTexts.push_back(pIndivRANKTEXT);   pIndivRANKTEXT->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("RankText"));

        // プレイヤー個別のBattleTimeText配置
        auto pIndivBTLTEXT = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        OBJ::Transform INDIV_BTL_TEXT = { {202, 59, 0.0f}, {0,0,0,0}, {baseX - 160 + TIMER_ADJUST, BASE_Y, 0.0f} };
        pIndivBTLTEXT->SetTransform(INDIV_BTL_TEXT);
        pIndivBTLTEXT->SetTransformTarget(INDIV_BTL_TEXT);
        m_vpPlayerBattleTexts.push_back(pIndivBTLTEXT);   pIndivBTLTEXT->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("BattleTimeText"));

        // プレイヤー名、タイム各種画像
        auto pMin = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pMin->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNum" + std::to_string(minutes)));
        pMin->SetTransform({ {32, 48, 0}, {0,0,0,0}, {baseX + TIMER_ADJUST, BASE_Y, 0} });
        pMin->SetTransformTarget(pMin->GetTransform());
        vpNums.push_back(pMin);

        auto pColon = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pColon->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNumCoron"));
        pColon->SetTransform({ {20, 48, 0}, {0,0,0,0}, {baseX + 36 + TIMER_ADJUST, BASE_Y, 0} });
        pColon->SetTransformTarget(pColon->GetTransform());
        vpNums.push_back(pColon);

        auto pSec10 = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pSec10->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNum" + std::to_string(seconds / 10)));
        pSec10->SetTransform({ {32, 48, 0}, {0,0,0,0}, {baseX + 60 + TIMER_ADJUST, BASE_Y, 0} });
        pSec10->SetTransformTarget(pSec10->GetTransform());
        vpNums.push_back(pSec10);

        auto pSec1 = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pSec1->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNum" + std::to_string(seconds % 10)));
        pSec1->SetTransform({ {32, 48, 0}, {0,0,0,0}, {baseX + 92 + TIMER_ADJUST, BASE_Y, 0} });
        pSec1->SetTransformTarget(pSec1->GetTransform());
        vpNums.push_back(pSec1);

        m_vvPlayerNumbers.push_back(vpNums);
    }
}

//============================================================================
// デストラクタ
//============================================================================
CSceneResult::~CSceneResult()
{

}

//============================================================================
// 更新
//============================================================================
void CSceneResult::Update()
{
	if (CInputManager::RefInstance().EnhancedEnter()) 
	{
		Change();
	}
}

void CSceneResult::Change()
{
	// 全オブジェクトに死亡フラグを立てる
	CObjectManager::RefInstance().SetDeathAll();

    //生存時間を破棄
    CPlayer::ClearAllSurvivalTimes();

	CSceneManager::RefInstance().ChangeScene(std::make_unique<CSceneTitle>());
}