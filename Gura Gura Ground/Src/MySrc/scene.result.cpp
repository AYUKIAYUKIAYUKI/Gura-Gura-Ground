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
    const float CENTER_X = SCREEN_W / 2.0f;
    const float CENTER_Y = SCREEN_H / 2.0f;

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
    const float RANK_Y_OFFSET = 40.0f;

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

    std::sort(sortList.begin(), sortList.end(),
        [](const PlayerRankInfo& a, const PlayerRankInfo& b) {
            return a.timeInt > b.timeInt || (a.timeInt == b.timeInt && a.idx < b.idx);
        }
    );
    std::vector<int> indexToRank(playerCount, 0);
    int rankNumber = 1;
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

    float titleTargetY = 100.0f;   // 規定の最終Y位置
    float titleStartY = -200.0f;   // 画面外(上)
    m_resultTitleTargetPos = DirectX::XMFLOAT3(CENTER_X, titleTargetY, 0);

    auto pResultTitle = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
    pResultTitle->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultTitle_Text"));
    OBJ::Transform TR = {
        DirectX::XMFLOAT3(483.0f, 200.0f, 0),           // サイズ
        DirectX::XMFLOAT4A(0,0,0,1),
        DirectX::XMFLOAT3(CENTER_X, titleStartY, 0)
    };
    pResultTitle->SetTransform(TR);
    pResultTitle->SetTransformTarget(TR);
    m_resultTitleIdx = static_cast<int>(m_vpNumbers.size());
    m_vpNumbers.push_back(pResultTitle);

    // WinText設定
    auto pWinText = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
    pWinText->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("WinText"));
    OBJ::Transform WIN_TEXT_TR = { {275, 146, 0}, {0,0,0,0}, {1500.0f, 300.0f, 0} };
    pWinText->SetTransform(WIN_TEXT_TR);
    pWinText->SetTransformTarget(WIN_TEXT_TR);
    // 最初アルファ0
    DirectX::XMFLOAT4 winTextCol = DirectX::XMFLOAT4(1, 1, 1, 0);
    pWinText->SetColTarget(winTextCol);
    pWinText->SetCol(winTextCol);
    m_winTextIdx = m_vpPlayerIcons.size();
    m_vpPlayerIcons.push_back(pWinText);

    int maxTimeInt = -1;
    for (size_t i = 0; i < playerCount; ++i) {
        if ((int)m_playerSurvivalTimes[i] > maxTimeInt)
            maxTimeInt = (int)m_playerSurvivalTimes[i];
    }
    std::vector<int> winners;
    for (size_t i = 0; i < playerCount; ++i) {
        if ((int)m_playerSurvivalTimes[i] == maxTimeInt) {
            winners.push_back((int)i);
        }
    }
    const float SINGLE_WIN_IMG_X = 1500.0f;
    const float SINGLE_WIN_IMG_Y = 500.0f;
    const float WIN_IMG_W = 664.0f;
    const float WIN_IMG_H = 133.0f;
    const float HALF_WIN_IMG_W = WIN_IMG_W / 2.0f;
    const float HALF_WIN_IMG_H = WIN_IMG_H / 2.0f;
    const float PLAYER_TXT_BASE_X = SINGLE_WIN_IMG_X;
    const float PLAYER_TXT_BASE_Y = SINGLE_WIN_IMG_Y;
    const float IMAGE_SPACING = 26.0f;

    // 勝者TextPlayer00X 初期化(スケール0, アルファ0)
    if (winners.size() == 1)
    {
        int winnerIndex = winners[0];
        std::string textPlayerTex = "TextPlayer00" + std::to_string(winnerIndex + 1);
        auto pText = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pText->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey(textPlayerTex));
        OBJ::Transform TEXT_PLAYER_TR = {
            DirectX::XMFLOAT3(WIN_IMG_W, WIN_IMG_H, 0),
            DirectX::XMFLOAT4A(0,0,0,1),
            DirectX::XMFLOAT3(PLAYER_TXT_BASE_X, PLAYER_TXT_BASE_Y, 0)
        };

        pText->SetTransform(TEXT_PLAYER_TR);
        pText->SetTransformTarget(TEXT_PLAYER_TR);
        DirectX::XMFLOAT4 col = DirectX::XMFLOAT4(1, 1, 1, 0);
        pText->SetColTarget(col);
        pText->SetCol(col);
        m_playerTextIdxs.push_back(m_vpPlayerTextImgs.size());
        m_vpPlayerTextImgs.push_back(pText);
    }
    else if (winners.size() > 1)
    {
        float totalH = winners.size() * HALF_WIN_IMG_H + (winners.size() - 1) * IMAGE_SPACING;
        float topY = PLAYER_TXT_BASE_Y - (totalH * 0.5f) + (HALF_WIN_IMG_H * 0.5f);
        for (size_t k = 0; k < winners.size(); ++k) {
            int winnerIndex = winners[k];
            std::string textPlayerTex = "TextPlayer00" + std::to_string(winnerIndex + 1);
            auto pText = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
            pText->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey(textPlayerTex));
            float posX = PLAYER_TXT_BASE_X;
            float posY = topY + k * (HALF_WIN_IMG_H + IMAGE_SPACING);
            OBJ::Transform TEXT_PLAYER_TR = {
                DirectX::XMFLOAT3(WIN_IMG_W, WIN_IMG_H, 0),
                DirectX::XMFLOAT4A(0,0,0,1),
                DirectX::XMFLOAT3(PLAYER_TXT_BASE_X, PLAYER_TXT_BASE_Y, 0)
            };

            pText->SetTransform(TEXT_PLAYER_TR);
            pText->SetTransformTarget(TEXT_PLAYER_TR);
            DirectX::XMFLOAT4 col = DirectX::XMFLOAT4(1, 1, 1, 0);
            pText->SetColTarget(col);
            pText->SetCol(col);
            m_playerTextIdxs.push_back(m_vpPlayerTextImgs.size());
            m_vpPlayerTextImgs.push_back(pText);
        }
    }

    // プレイヤー数分UI表示(アニメ用: 全画像アルファ0で初期化)
    for (size_t playerIdx = 0; playerIdx < playerCount; ++playerIdx)
    {
        std::vector<CHud*> vpNums;
        float surv = m_playerSurvivalTimes[playerIdx];
        int totalSec = static_cast<int>(surv);
        int minutes = totalSec / 60;
        int seconds = totalSec % 60;

        float baseX = leftmostX + playerIdx * TIMER_INTERVAL;

        // PlayerLight画像
        std::string playerLightTex = "PlayerLight00" + std::to_string(playerIdx + 1);
        auto pLight = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pLight->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey(playerLightTex));
        OBJ::Transform TF_Light = { {705, 610, 0}, {0,0,0,0}, {baseX + 10.0f, 910.0f, 0} };
        pLight->SetTransform(TF_Light);
        pLight->SetTransformTarget(TF_Light);
        DirectX::XMFLOAT4 col = DirectX::XMFLOAT4(1, 1, 1, 0); // アルファ0
        pLight->SetColTarget(col);
        pLight->SetCol(col);
        m_vpPlayerLights.push_back(pLight);

        // IconPlayer00X画像
        std::string iconTex = "IconPlayer00" + std::to_string(playerIdx + 1);
        auto pIcon = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pIcon->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey(iconTex));
        float iconX = baseX - 220 + TIMER_ADJUST;
        OBJ::Transform ICON_TR = { {ICON_SIZE_W, ICON_SIZE_H, 0}, {0,0,0,0}, {iconX, 800.0f, 0} };
        pIcon->SetTransform(ICON_TR);
        pIcon->SetTransformTarget(ICON_TR);
        pIcon->SetColTarget(col);
        pIcon->SetCol(col);
        m_vpPlayerIcons.push_back(pIcon);

        // Rank画像
        int rankImageNumber = indexToRank[playerIdx];
        std::string rankTex = "Rank_" + std::to_string(rankImageNumber);
        auto pRank = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        OBJ::Transform RANK_TR = { {134, 94, 0}, {0,0,0,0}, {baseX + 100, BASE_Y - 100.0f, 0} };
        pRank->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey(rankTex));
        pRank->SetTransform(RANK_TR);
        pRank->SetTransformTarget(RANK_TR);
        pRank->SetColTarget(col);
        pRank->SetCol(col);
        m_vpPlayerRankImgs.push_back(pRank);

        // RankText画像
        auto pIndivRANKTEXT = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        OBJ::Transform INDIV_RANK_TEXT = { {107, 59, 0.0f}, {0,0,0,0}, {baseX - 210 + TIMER_ADJUST, BASE_Y - 90.0f, 0.0f} };
        pIndivRANKTEXT->SetTransform(INDIV_RANK_TEXT);
        pIndivRANKTEXT->SetTransformTarget(INDIV_RANK_TEXT);
        pIndivRANKTEXT->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("RankText"));
        pIndivRANKTEXT->SetColTarget(col);
        pIndivRANKTEXT->SetCol(col);
        m_vpPlayerBattleTexts.push_back(pIndivRANKTEXT);

        // BattleTimeText画像
        auto pIndivBTLTEXT = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        OBJ::Transform INDIV_BTL_TEXT = { {202, 59, 0.0f}, {0,0,0,0}, {baseX - 160 + TIMER_ADJUST, BASE_Y, 0.0f} };
        pIndivBTLTEXT->SetTransform(INDIV_BTL_TEXT);
        pIndivBTLTEXT->SetTransformTarget(INDIV_BTL_TEXT);
        pIndivBTLTEXT->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("BattleTimeText"));
        pIndivBTLTEXT->SetColTarget(col);
        pIndivBTLTEXT->SetCol(col);
        m_vpPlayerBattleTexts.push_back(pIndivBTLTEXT);

        // 分・コロン・秒の数字HUD
        auto pMin = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pMin->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNum" + std::to_string(minutes)));
        pMin->SetTransform({ {32, 48, 0}, {0,0,0,0}, {baseX + TIMER_ADJUST, BASE_Y, 0} });
        pMin->SetTransformTarget(pMin->GetTransform());
        pMin->SetColTarget(col);
        pMin->SetCol(col);
        vpNums.push_back(pMin);

        auto pColon = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pColon->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNumCoron"));
        pColon->SetTransform({ {20, 48, 0}, {0,0,0,0}, {baseX + 36 + TIMER_ADJUST, BASE_Y, 0} });
        pColon->SetTransformTarget(pColon->GetTransform());
        pColon->SetColTarget(col);
        pColon->SetCol(col);
        vpNums.push_back(pColon);

        auto pSec10 = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pSec10->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNum" + std::to_string(seconds / 10)));
        pSec10->SetTransform({ {32, 48, 0}, {0,0,0,0}, {baseX + 60 + TIMER_ADJUST, BASE_Y, 0} });
        pSec10->SetTransformTarget(pSec10->GetTransform());
        pSec10->SetColTarget(col);
        pSec10->SetCol(col);
        vpNums.push_back(pSec10);

        auto pSec1 = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pSec1->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNum" + std::to_string(seconds % 10)));
        pSec1->SetTransform({ {32, 48, 0}, {0,0,0,0}, {baseX + 92 + TIMER_ADJUST, BASE_Y, 0} });
        pSec1->SetTransformTarget(pSec1->GetTransform());
        pSec1->SetColTarget(col);
        pSec1->SetCol(col);
        vpNums.push_back(pSec1);

        m_vvPlayerNumbers.push_back(vpNums);
    }

    m_animPhase = ANIM_PHASE::TITLE_MOVE;
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
    float deltaT = 1.0f / 60.0f; // フレーム時間(仮/必要に応じて時間差対応)

    switch (m_animPhase)
    {
    case ANIM_PHASE::TITLE_MOVE:
        if (m_resultTitleIdx >= 0 && m_resultTitleIdx < (int)m_vpNumbers.size()) {
            auto pHud = m_vpNumbers[m_resultTitleIdx];
            // 現在位置
            OBJ::Transform tr = pHud->GetTransform();
            // 線形補間で降下
            float speed = 5.0f;
            float nextY = tr.Pos.y + speed;
            if (nextY >= m_resultTitleTargetPos.y) {
                nextY = m_resultTitleTargetPos.y;
                m_animPhase = ANIM_PHASE::TITLE_WAIT; // タイトル着地後待機状態に入る
            }
            tr.Pos.y = nextY;
            pHud->SetTransform(tr);
            pHud->SetTransformTarget(tr);
        }
        break;
    case ANIM_PHASE::TITLE_WAIT:
        m_animTimer += deltaT;
        if (m_animTimer > 1.5f) {
            m_animPhase = ANIM_PHASE::WIN_TEXT_FADEIN;
            m_animTimer = 0.0f;
        }
        break;
    case ANIM_PHASE::WIN_TEXT_FADEIN:
    {
        m_winTextAlpha += 8.0f; // 8ずつ加算(約32フレで255)
        if (m_winTextAlpha > 255.0f) m_winTextAlpha = 255.0f;
        float a = m_winTextAlpha / 255.0f;

        if (m_winTextIdx >= 0 && m_winTextIdx < (int)m_vpPlayerIcons.size()) 
        {
            auto pHud = m_vpPlayerIcons[m_winTextIdx];
            DirectX::XMFLOAT4 col = pHud->GetColTarget();
            col.w = a;
            pHud->SetColTarget(col);
            pHud->SetCol(col); // 即反映
        }

        if (m_winTextAlpha >= 255.0f) {
            m_animTimer = 0.0f;
            m_animPhase = ANIM_PHASE::WIN_WAIT;
        }
    }
    break;
    case ANIM_PHASE::WIN_WAIT:
        m_animTimer += deltaT;
        if (m_animTimer > 3.0f) {
            m_animTimer = 0.0f;
            m_animPhase = ANIM_PHASE::PLAYER_TEXT_SCALEUP;
        }
        break;
    case ANIM_PHASE::PLAYER_TEXT_SCALEUP:
    {
        m_playerTextScale += 0.04f;
        if (m_playerTextScale > 1.0f) m_playerTextScale = 1.0f;
        // 対応HUDすべて対応
        for (auto idx : m_playerTextIdxs) {
            if (idx >= 0 && idx < (int)m_vpPlayerTextImgs.size()) {
                auto pHud = m_vpPlayerTextImgs[idx];
                OBJ::Transform tf = pHud->GetTransformTarget();
                //tf.scale.x = m_playerTextScale;
                //tf.scale.y = m_playerTextScale;
                pHud->SetTransformTarget(tf);
                // α表示も1に
                DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = (m_playerTextScale < 1.0f) ? m_playerTextScale : 1.0f;
                pHud->SetColTarget(col);
                pHud->SetCol(col);
            }
        }
        if (m_playerTextScale >= 1.0f) {
            m_animPhase = ANIM_PHASE::PLAYER_WAIT;
            m_animTimer = 0.0f;
        }
    }
    break;
    case ANIM_PHASE::PLAYER_WAIT:
        m_animTimer += deltaT;
        if (m_animTimer > 1.0f) {
            m_animPhase = ANIM_PHASE::SHOW_OTHERS;
            m_otherAlpha = 0.0f;
        }
        break;
    case ANIM_PHASE::SHOW_OTHERS:
        m_otherAlpha += 10.0f;
        if (m_otherAlpha > 255.0f) m_otherAlpha = 255.0f;
        {
            float a = m_otherAlpha / 255.0f;
            // BG以外すべてをフェードイン
            // 例: m_vpPlayerIcons, m_vpPlayerLights, m_vpPlayerBattleTexts, m_vpPlayerRankImgs など
            for (auto& pHud : m_vpPlayerLights) {
                if (pHud) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = a; pHud->SetColTarget(col); pHud->SetCol(col); }
            }
            for (auto& pHud : m_vpPlayerIcons) {
                if (pHud && pHud != m_vpPlayerIcons[m_winTextIdx]) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = a; pHud->SetColTarget(col); pHud->SetCol(col); }
            }
            for (auto& pHud : m_vpPlayerBattleTexts) {
                if (pHud) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = a; pHud->SetColTarget(col); pHud->SetCol(col); }
            }
            for (auto& pHud : m_vpPlayerRankImgs) {
                if (pHud) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = a; pHud->SetColTarget(col); pHud->SetCol(col); }
            }
            for (auto& vv : m_vvPlayerNumbers) {
                for (auto& pHud : vv) {
                    if (pHud) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = a; pHud->SetColTarget(col); pHud->SetCol(col); }
                }
            }
        }
        if (m_otherAlpha >= 255.0f) {
            m_animPhase = ANIM_PHASE::FINISHED;
        }
        break;
    case ANIM_PHASE::FINISHED:
        // 通常のUpdate分岐や入力受付へ
        if (CInputManager::RefInstance().EnhancedEnter()) {
            Change();
        }
        break;
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