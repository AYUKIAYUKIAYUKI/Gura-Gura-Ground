//============================================================================
// 
// リザルトシーン実装ファイル [scene.result.cpp]
// Author : Sohta Kuki
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
#include <enemy1.h>
#include <effect.manager.h>

extern size_t gConnectedHumanPlayerNum = {};


//============================================================================
// コンストラクタ
//============================================================================
CSceneResult::CSceneResult(const std::vector<float>& times, int nHuman, int nCPU)
    : m_playerSurvivalTimes(times), m_nHumanPlayerNum(nHuman), m_nCPUNum(nCPU)
{
    //一旦既存の音声を全て止める
    CSoundManger::RefInstance().StopAll();

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

    // BG暗転カラー設定
    if (m_pBackground)
    {
        float dark = 0.25f;
        DirectX::XMFLOAT4 color(dark, dark, dark, 1.0f);
        m_pBackground->SetCol(color);
        m_pBackground->SetColTarget(color);
        m_bgDarkRatio = 0.0f;
    }

    const float BASE_Y = 980.0f;

    const int TIMER_ADJUST = 50;
    const float TIMER_INTERVAL = 435.0f;
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
        int time = static_cast<int>(m_playerSurvivalTimes[i]);
        int cappedTime = (time > 599) ? 599 : time;
        sortList.push_back(PlayerRankInfo{ (int)i, cappedTime, m_playerSurvivalTimes[i] });
    }

    std::sort(sortList.begin(), sortList.end(),
        [](const PlayerRankInfo& a, const PlayerRankInfo& b)
        {
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

    float titleTargetY = 100.0f;
    float titleStartY = -200.0f;
    m_resultTitleTargetPos = DirectX::XMFLOAT3(CENTER_X, titleTargetY, 0);

    auto pResultTitle = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
    pResultTitle->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultTitle_Text"));
    OBJ::Transform TR = {
        DirectX::XMFLOAT3(483.0f, 200.0f, 0),
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
    DirectX::XMFLOAT4 winTextCol = DirectX::XMFLOAT4(1, 1, 1, 0);
    pWinText->SetColTarget(winTextCol);
    pWinText->SetCol(winTextCol);
    m_winTextIdx = m_vpPlayerIcons.size();
    m_vpPlayerIcons.push_back(pWinText);

    // 勝者判定
    int maxTimeInt = -1;
    for (size_t i = 0; i < playerCount; ++i)
    {
        if ((int)m_playerSurvivalTimes[i] > maxTimeInt)
            maxTimeInt = (int)m_playerSurvivalTimes[i];
    }

    for (size_t i = 0; i < playerCount; ++i)
    {
        if ((int)m_playerSurvivalTimes[i] == maxTimeInt) {
            m_winners.push_back((int)i);
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

    if (m_winners.size() == 1)
    {
        int winnerIndex = m_winners[0];
        bool isHumanWinner = (winnerIndex < m_nHumanPlayerNum);

        std::string modelName;

        if (isHumanWinner) {
            
            modelName = "Player_" + std::to_string(winnerIndex + 1);
        }
        else {
            modelName = "Test";
        }

        int dispIdx = isHumanWinner ? (winnerIndex + 1) : (winnerIndex + 1 - m_nHumanPlayerNum);
        std::string textPlayerTex = isHumanWinner
            ? "TextPlayer00" + std::to_string(dispIdx)
            : "TextCPU00" + std::to_string(dispIdx);
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
    else if (m_winners.size() > 1)
    {
        float totalH = m_winners.size() * HALF_WIN_IMG_H + (m_winners.size() - 1) * IMAGE_SPACING;
        float topY = PLAYER_TXT_BASE_Y - (totalH * 0.5f) + (HALF_WIN_IMG_H * 0.5f);
        for (size_t k = 0; k < m_winners.size(); ++k) {
            int winnerIndex = m_winners[k];
            bool isHumanWinner = (winnerIndex < m_nHumanPlayerNum);
            int dispIdx = isHumanWinner ? (winnerIndex + 1) : (winnerIndex + 1 - m_nHumanPlayerNum);
            std::string textPlayerTex = isHumanWinner
                ? "TextPlayer00" + std::to_string(dispIdx)
                : "TextCPU00" + std::to_string(dispIdx);
            auto pText = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
            pText->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey(textPlayerTex));
            float posX = PLAYER_TXT_BASE_X;
            float posY = topY + k * (HALF_WIN_IMG_H + IMAGE_SPACING);
            OBJ::Transform TEXT_PLAYER_TR = {
                DirectX::XMFLOAT3(HALF_WIN_IMG_W, HALF_WIN_IMG_H, 0),
                DirectX::XMFLOAT4A(0,0,0,1),
                DirectX::XMFLOAT3(posX, posY, 0)
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


    // プレイヤーとCPU全員分のUI生成
    for (size_t playerIdx = 0; playerIdx < playerCount; ++playerIdx)
    {
        std::vector<CHud*> vpNums;
        float surv = m_playerSurvivalTimes[playerIdx];
        int totalSec = static_cast<int>(surv);
        int minutes, seconds;

		// totalSec599以上の場合強制的に9:59にする
        if (totalSec >= 599) {
            minutes = 9;
            seconds = 59;
        }
        else {
            minutes = totalSec / 60;
            seconds = totalSec % 60;
        }

        float baseX = leftmostX + playerIdx * TIMER_INTERVAL;
        auto clamp_num = [](int x) { return (x < 0) ? 0 : ((x > 9) ? 9 : x); };

        // プレイヤーかCPUかを判定
        bool isHuman = (playerIdx < m_nHumanPlayerNum);
        int dispIdx = isHuman ? (playerIdx + 1) : (playerIdx + 1 - m_nHumanPlayerNum);

        // PlayerLight画像
        std::string lightTex = isHuman
            ? "PlayerLight00" + std::to_string(dispIdx)
            : "CPULight00" + std::to_string(dispIdx);
        auto pLight = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pLight->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey(lightTex));
        OBJ::Transform TF_Light = { {705, 610, 0}, {0,0,0,0}, {baseX + 10.0f, 910.0f, 0} };
        pLight->SetTransform(TF_Light);
        pLight->SetTransformTarget(TF_Light);
        DirectX::XMFLOAT4 col = DirectX::XMFLOAT4(1, 1, 1, 0);
        pLight->SetColTarget(col);
        pLight->SetCol(col);
        m_vpPlayerLights.push_back(pLight);

        // プレイヤーアイコン表示
        std::string iconTex = isHuman
            ? "IconPlayer00" + std::to_string(dispIdx)
            : "IconCPU00" + std::to_string(dispIdx);
        auto pIcon = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pIcon->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey(iconTex));
        float iconX = baseX - 220 + TIMER_ADJUST;

        OBJ::Transform ICON_TR;
        if (!isHuman) {
            // CPUアイコンのみサイズを変更する
            ICON_TR = { {124.0f, ICON_SIZE_H, 0}, {0,0,0,0}, {iconX, 800.0f, 0} };
        }
        else 
        {
            ICON_TR = { {ICON_SIZE_W, ICON_SIZE_H, 0}, {0,0,0,0}, {iconX, 800.0f, 0} };
        }

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

        // RankText
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
        int min_num = clamp_num(minutes);
        pMin->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNum" + std::to_string(min_num)));
        pMin->SetTransform({ {64, 64, 0}, {0,0,0,0}, {baseX + TIMER_ADJUST, BASE_Y, 0} });
        pMin->SetTransformTarget(pMin->GetTransform());
        pMin->SetColTarget(col);
        pMin->SetCol(col);
        vpNums.push_back(pMin);

        auto pColon = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        pColon->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNumCoron"));
        pColon->SetTransform({ {64, 64, 0}, {0,0,0,0}, {baseX + 36 + TIMER_ADJUST, BASE_Y, 0} });
        pColon->SetTransformTarget(pColon->GetTransform());
        pColon->SetColTarget(col);
        pColon->SetCol(col);
        vpNums.push_back(pColon);

        auto pSec10 = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        int sec10_num = clamp_num(seconds / 10);
        pSec10->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNum" + std::to_string(sec10_num)));
        pSec10->SetTransform({ {64, 64, 0}, {0,0,0,0}, {baseX + 60 + TIMER_ADJUST, BASE_Y, 0} });
        pSec10->SetTransformTarget(pSec10->GetTransform());
        pSec10->SetColTarget(col);
        pSec10->SetCol(col);
        vpNums.push_back(pSec10);

        auto pSec1 = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        int sec1_num = clamp_num(seconds % 10);
        pSec1->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultNum" + std::to_string(sec1_num)));
        pSec1->SetTransform({ {64, 64, 0}, {0,0,0,0}, {baseX + 92 + TIMER_ADJUST, BASE_Y, 0} });
        pSec1->SetTransformTarget(pSec1->GetTransform());
        pSec1->SetColTarget(col);
        pSec1->SetCol(col);
        vpNums.push_back(pSec1);

        m_vvPlayerNumbers.push_back(vpNums);
    }

    auto pSkipText = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
    pSkipText->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultSceneSkipText"));
    OBJ::Transform SKIP_TEXT_TR = { {169, 44, 0}, {0,0,0,0}, {1825.0f, 1050.0f, 0} };
    pSkipText->SetTransform(SKIP_TEXT_TR);
    pSkipText->SetTransformTarget(SKIP_TEXT_TR);
    DirectX::XMFLOAT4 skipCol = DirectX::XMFLOAT4(1, 1, 1, 0);
    pSkipText->SetCol(skipCol);
    pSkipText->SetColTarget(skipCol);
    m_vpSkipTexts.push_back(pSkipText);
    m_vSkipTextAlpha.push_back(0.0f);

    auto pTitleText = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
    pTitleText->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("ResultSceneTitleText"));
    OBJ::Transform TITLE_TEXT_TR = { {169, 44, 0}, {0,0,0,0}, {1825.0f, 1050.0f, 0} };
    pTitleText->SetTransform(TITLE_TEXT_TR);
    pTitleText->SetTransformTarget(TITLE_TEXT_TR);
    DirectX::XMFLOAT4 titleCol = DirectX::XMFLOAT4(1, 1, 1, 0);
    pTitleText->SetCol(titleCol);
    pTitleText->SetColTarget(titleCol);
    m_vpTitleTexts.push_back(pTitleText);
    m_vTitleTextAlpha.push_back(0.0f);

    float skipX = SKIP_TEXT_TR.Pos.x;
    float skipY = SKIP_TEXT_TR.Pos.y;
    float skipW = SKIP_TEXT_TR.Size.x;
    float skipH = SKIP_TEXT_TR.Size.y;
    float margin = 15.0f;
    float buttonW = 44.0f;
    float buttonH = 44.0f;

    float buttonAX = skipX - skipW / 2.0f - margin - buttonW / 2.0f;
    float buttonAY = skipY;
    OBJ::Transform BUTTONA_TR = { {buttonW, buttonH, 0}, {0,0,0,0}, {buttonAX, buttonAY, 0} };

    auto pButtonA = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
    pButtonA->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Button_A"));
    pButtonA->SetTransform(BUTTONA_TR);
    pButtonA->SetTransformTarget(BUTTONA_TR);
    DirectX::XMFLOAT4 buttonCol = DirectX::XMFLOAT4(1, 1, 1, 0);
    pButtonA->SetCol(buttonCol);
    pButtonA->SetColTarget(buttonCol);
    m_vpSkipButtonA.push_back(pButtonA);
    m_vSkipButtonAAlpha.push_back(0.0f);

    auto pTitleButtonA = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
    pTitleButtonA->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Button_A"));
    pTitleButtonA->SetTransform(BUTTONA_TR);
    pTitleButtonA->SetTransformTarget(BUTTONA_TR);
    DirectX::XMFLOAT4 btnCol = DirectX::XMFLOAT4(1, 1, 1, 0);
    pTitleButtonA->SetCol(btnCol);
    pTitleButtonA->SetColTarget(btnCol);
    m_vpTitleButtonA.push_back(pTitleButtonA);
    m_vTitleButtonAAlpha.push_back(0.0f);


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
    float deltaT = 1.0f / 60.0f; // フレーム時間
    float fireworkTimer = 0.0f;

    // カメラ高さ制御ロジック
    if (m_animPhase < ANIM_PHASE::PLAYER_TEXT_SCALEUP) 
    {
        m_cameraHeight = -15.0f;
    }

    else if (m_animPhase == ANIM_PHASE::PLAYER_TEXT_SCALEUP) 
    {
        if (m_cameraHeight < 0.0f) 
        {
            float cameraUpSpeed = 0.5f; //カメラ移動速度
            m_cameraHeight += cameraUpSpeed;
            if (m_cameraHeight > 0.0f) m_cameraHeight = 0.0f;
        }
    }

    else if (m_animPhase > ANIM_PHASE::PLAYER_TEXT_SCALEUP) 
    {
        m_cameraHeight = 0.0f;
    }

    // カメラ位置反映
    CCamera* pCamera = CRenderer::RefInstance().GetCamera();
    if (pCamera) {
        DirectX::XMFLOAT3 pos = pCamera->GetPos();
        DirectX::XMFLOAT3 rot = pCamera->GetRot();
        pos.x = 0.0f;
        pos.y = m_cameraHeight;
        pos.z = 0.0f;
        rot.x = 0.0f; rot.y = 0.0f; rot.z = 0.0f;
        pCamera->SetPos(pos);
        pCamera->SetRot(rot);
    }

    bool drawBeam = true;

    if (m_animPhase >= ANIM_PHASE::TITLE_WAIT && m_animPhase != ANIM_PHASE::FINISHED)
    {
        if (CInputManager::RefInstance().EnhancedEnter())
        {
            ForceToFinished();
            return; // それ以降のアニメフェーズごとの処理をスキップ
        }
    }

    if (m_pBackground)
    {
        // 背景インアニメーション
        float from = 0.25f; // 暗
        float to = 1.0f;  // 明
        float speed = 0.03f; // 明るくなる速度
        bool shouldBrighten = false;

        // PLAYER_TEXT_SCALEUPで明るくする
        if (m_animPhase == ANIM_PHASE::PLAYER_TEXT_SCALEUP ||
            m_animPhase == ANIM_PHASE::PLAYER_WAIT ||
            m_animPhase == ANIM_PHASE::SHOW_OTHERS ||
            m_animPhase == ANIM_PHASE::FINISHED)
        {
            shouldBrighten = true;
        }

        if (shouldBrighten)
        {
            if (m_bgDarkRatio < 1.0f)
                m_bgDarkRatio += speed;
            if (m_bgDarkRatio > 1.0f)
                m_bgDarkRatio = 1.0f;
        }
        else
        {
            m_bgDarkRatio = 0.0f;
        }

        float colorLerp = from * (1.0f - m_bgDarkRatio) + to * m_bgDarkRatio;
        DirectX::XMFLOAT4 newColor(colorLerp, colorLerp, colorLerp, 1.0f);
        m_pBackground->SetCol(newColor);
        m_pBackground->SetColTarget(newColor);
    }


    static auto lastFireworkTime = std::chrono::steady_clock::now();

    // PLAYER_WAIT以降で花火を生成
    if (m_animPhase >= ANIM_PHASE::PLAYER_WAIT)
    {
        auto now = std::chrono::steady_clock::now();
        float interval = 0.5f; // 秒

        std::chrono::duration<float> elapsed = now - lastFireworkTime;
        if (elapsed.count() >= interval)
        {
            lastFireworkTime = now;
            float randX = -6.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (12.0f)));
            CEffect::Create(CEffectManager::TAG_FIREWORKS_SINGLE, { randX, -23.5f, 0.0f });
        }
    }
    else
    {
        // フェーズがそれ以前のときは最新の時刻でリセット
        lastFireworkTime = std::chrono::steady_clock::now();
    }

    switch (m_animPhase)
    {
    case ANIM_PHASE::TITLE_MOVE:
        if (m_resultTitleIdx >= 0 && m_resultTitleIdx < (int)m_vpNumbers.size())
        {
            if (!m_StartBuzzer)
            {
                CSoundManger::RefInstance().Play("Buzzer", false, -0.5f, 1.0f);

                m_StartBuzzer = true;
            }

            auto pHud = m_vpNumbers[m_resultTitleIdx];
            OBJ::Transform tr = pHud->GetTransform();
            float speed = 5.0f;
            float nextY = tr.Pos.y + speed;
            if (nextY >= m_resultTitleTargetPos.y) 
            {
                nextY = m_resultTitleTargetPos.y;
                m_animPhase = ANIM_PHASE::TITLE_WAIT;
            }
            tr.Pos.y = nextY;
            pHud->SetTransform(tr);
            pHud->SetTransformTarget(tr);
        }
        break;
    case ANIM_PHASE::TITLE_WAIT:
        m_animTimer += deltaT;
        if (m_animTimer > 1.5f) {
            CSoundManger::RefInstance().Stop("Buzzer");
            m_animPhase = ANIM_PHASE::WIN_TEXT_FADEIN;
            m_animTimer = 0.0f;
        }
        break;
    case ANIM_PHASE::WIN_TEXT_FADEIN:
    {
        m_winTextAlpha += 8.0f;
        if (m_winTextAlpha > 255.0f) m_winTextAlpha = 255.0f;
        float a = m_winTextAlpha / 255.0f;

        if (m_winTextIdx >= 0 && m_winTextIdx < (int)m_vpPlayerIcons.size())
        {
            auto pHud = m_vpPlayerIcons[m_winTextIdx];
            DirectX::XMFLOAT4 col = pHud->GetColTarget();
            col.w = a;
            pHud->SetColTarget(col);
            pHud->SetCol(col);
        }

        if (m_winTextAlpha >= 255.0f) {
            m_animTimer = 0.0f;
            CSoundManger::RefInstance().Play("Drumroll", false, 0.0f, 1.0f);
            m_animPhase = ANIM_PHASE::WIN_WAIT;
        }
    }
    break;
    case ANIM_PHASE::WIN_WAIT:
        m_animTimer += deltaT;
        if (m_animTimer > 3.0f)
        {
            m_animTimer = 0.0f;
            m_animPhase = ANIM_PHASE::PLAYER_TEXT_SCALEUP;
        }
        break;
    case ANIM_PHASE::PLAYER_TEXT_SCALEUP:
    {
        if (!m_WinnerModelAppeared)
        {
            const float baseX = -12.0f;
            const float spacing = 6.0f; // 勝者同士の間隔
            size_t winCount = m_winners.size();

            for (size_t k = 0; k < winCount; ++k)
            {
                int winnerIndex = m_winners[k];
                bool isHumanWinner = (winnerIndex < m_nHumanPlayerNum);
                std::string modelName;

                if (isHumanWinner) {
                    modelName = "Player_" + std::to_string(winnerIndex + 1);
                }
                else {
                    modelName = "Test";
                }

                // 横並び配置用にX座標をずらす
                float offsetX = baseX + spacing * ((int)k - (int)(winCount - 1) / 2.0f);

                auto model = CObjectManager::CreateShare<CPhysicsModel>(
                    [modelName, offsetX](CPhysicsModel* p) -> bool
                    {
                        p->SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey(modelName));
                        OBJ::Transform tf;
                        tf.Pos = { offsetX, 0.0f, 0.0f };
                        tf.Rot = { 0.0f, 0.0f, 0.0f, 1.0f };
                        p->SetTransform(tf);
                        p->FactoryCollider(1.0f, 1.0f, 1.0f);
                        return true;
                    },
                    OBJ::TYPE::NONE, OBJ::LAYER::FRONT
                        );
               
                if (k == 0) m_spTestModel = model;
            }

            m_WinnerModelAppeared = true;
        }

        m_playerTextScale += 0.04f;
        if (m_playerTextScale > 1.0f) m_playerTextScale = 1.0f;
        for (auto idx : m_playerTextIdxs) {
            if (idx >= 0 && idx < (int)m_vpPlayerTextImgs.size()) {
                auto pHud = m_vpPlayerTextImgs[idx];
                OBJ::Transform tf = pHud->GetTransformTarget();
                pHud->SetTransformTarget(tf);
                DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = (m_playerTextScale < 1.0f) ? m_playerTextScale : 1.0f;
                pHud->SetColTarget(col);
                pHud->SetCol(col);
            }
        }
        if (!m_beamLightAppeared)
        {
            m_beamLightAppeared = true;
            m_vpBeamLight.clear();
            for (int i = 0; i < 3; ++i) {
                float time = 5.0f + i;
                DirectX::XMFLOAT2 pos = { 0.3f * (i == 0 ? 1 : i == 1 ? -1 : 0), 0.75f };
                auto pBeam = CObjectManager::CreateRaw<CBeamLight>(OBJ::TYPE::NONE, OBJ::LAYER::FRONT);
                pBeam->SetPos(pos);
                pBeam->SetTime(time);
                pBeam->SetEnableTime(true);
                m_vpBeamLight.push_back(pBeam);
                CSoundManger::RefInstance().Stop("Drumroll");
                CSoundManger::RefInstance().Play("Drumroll_Finish", false, 0.0f, 1.0f);

            }
        }
        if (m_playerTextScale >= 1.0f && m_cameraHeight >= 0.0f)
        {
            m_animPhase = ANIM_PHASE::PLAYER_WAIT;
            m_animTimer = 0.0f;
        }
    }
    break;
    case ANIM_PHASE::PLAYER_WAIT:

        if (!m_kirakiraEffectCreated) 
        {
            if (m_winners.size() == 1)
            {
                CEffect::Create(CEffectManager::TAG_SPARKLE, { 21.0f, 1.0f, 0.0f }, {}, { 0.5f });
                CEffect::Create(CEffectManager::TAG_SPARKLE, { 12.6f, 2.8f, 0.0f }, {}, { 0.5f });
                CEffect::Create(CEffectManager::TAG_SPARKLE, { 6.5f, -0.8f, 0.0f }, {}, { 0.5f });
            }

            else if (m_winners.size() > 1)
            {
                CEffect::Create(CEffectManager::TAG_SPARKLE, { 17.8f, 1.0f, 0.0f }, {}, { 0.5f });
                CEffect::Create(CEffectManager::TAG_SPARKLE, { 12.6f, 3.4f, 0.0f }, {}, { 0.5f });
                CEffect::Create(CEffectManager::TAG_SPARKLE, { 10.5f, -0.8f, 0.0f }, {}, { 0.5f });
            }

            CSoundManger::RefInstance().Play("BGM_RESULT", false, 0.0f, 1.0f);
            m_kirakiraEffectCreated = true;
        }

        m_animTimer += deltaT;

        if (m_animTimer > 1.0f) 
        {
            m_animPhase = ANIM_PHASE::SHOW_OTHERS;
            m_otherAlpha = 0.0f;
        }
        break;
    case ANIM_PHASE::SHOW_OTHERS:
        m_otherAlpha += 10.0f;
        if (m_otherAlpha > 255.0f) m_otherAlpha = 255.0f;
        {
            float a = m_otherAlpha / 255.0f;
            for (auto& pHud : m_vpPlayerLights) 
            {
                if (pHud) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = a; pHud->SetColTarget(col); pHud->SetCol(col); }
            }
            for (auto& pHud : m_vpPlayerIcons) 
            {
                if (pHud && pHud != m_vpPlayerIcons[m_winTextIdx]) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = a; pHud->SetColTarget(col); pHud->SetCol(col); }
            }
            for (auto& pHud : m_vpPlayerBattleTexts)
            {
                if (pHud) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = a; pHud->SetColTarget(col); pHud->SetCol(col); }
            }
            for (auto& pHud : m_vpPlayerRankImgs)
            {
                if (pHud) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = a; pHud->SetColTarget(col); pHud->SetCol(col); }
            }
            for (auto& vv : m_vvPlayerNumbers)
            {
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
        if (CInputManager::RefInstance().EnhancedEnter())
        {
            Change();
        }
        break;
    }

    // スキップテキストフェードイン
    for (size_t i = 0; i < m_vpSkipTexts.size(); ++i)
    {
        CHud* pSkip = m_vpSkipTexts[i];
        CHud* pButtonA = (i < m_vpSkipButtonA.size()) ? m_vpSkipButtonA[i] : nullptr;
        if (!pSkip || !pButtonA) continue;

        if (m_animPhase >= ANIM_PHASE::TITLE_WAIT && m_animPhase != ANIM_PHASE::FINISHED)
        {
            // 同じフェードイン値で管理しましょうか
            float fadeRate = 10.0f;
            m_vSkipTextAlpha[i] += fadeRate;
            if (m_vSkipTextAlpha[i] > 255.0f) m_vSkipTextAlpha[i] = 255.0f;

            float fadeAlpha = m_vSkipTextAlpha[i] / 255.0f;

            DirectX::XMFLOAT4 skipCol = pSkip->GetColTarget();
            skipCol.w = fadeAlpha;
            pSkip->SetColTarget(skipCol);
            pSkip->SetCol(skipCol);

            DirectX::XMFLOAT4 btnCol = pButtonA->GetColTarget();
            btnCol.w = fadeAlpha;
            pButtonA->SetColTarget(btnCol);
            pButtonA->SetCol(btnCol);
        }
        else
        {
            // 非表示
            DirectX::XMFLOAT4 skipCol = pSkip->GetColTarget();
            skipCol.w = 0.0f;
            pSkip->SetColTarget(skipCol);
            pSkip->SetCol(skipCol);
            m_vSkipTextAlpha[i] = 0.0f;

            DirectX::XMFLOAT4 btnCol = pButtonA->GetColTarget();
            btnCol.w = 0.0f;
            pButtonA->SetColTarget(btnCol);
            pButtonA->SetCol(btnCol);
        }
    }

    for (size_t i = 0; i < m_vpTitleTexts.size(); ++i)
    {
        CHud* pTitle = m_vpTitleTexts[i];
        CHud* pButtonA = (i < m_vpTitleButtonA.size()) ? m_vpTitleButtonA[i] : nullptr;
        if (!pTitle) continue;
        // タイトルテキストのフェードイン
        if (m_animPhase == ANIM_PHASE::FINISHED)
        {
            // タイトルテキスト
            m_vTitleTextAlpha[i] += 10.0f;
            if (m_vTitleTextAlpha[i] > 255.0f) m_vTitleTextAlpha[i] = 255.0f;
            DirectX::XMFLOAT4 titleCol = pTitle->GetColTarget();
            titleCol.w = m_vTitleTextAlpha[i] / 255.0f;
            pTitle->SetColTarget(titleCol);
            pTitle->SetCol(titleCol);

            // タイトルAボタン
            if (pButtonA) {
                m_vTitleButtonAAlpha[i] += 10.0f;
                if (m_vTitleButtonAAlpha[i] > 255.0f) m_vTitleButtonAAlpha[i] = 255.0f;
                DirectX::XMFLOAT4 btnCol = pButtonA->GetColTarget();
                btnCol.w = m_vTitleButtonAAlpha[i] / 255.0f;
                pButtonA->SetColTarget(btnCol);
                pButtonA->SetCol(btnCol);
            }
        }
        else
        {
            DirectX::XMFLOAT4 titleCol = pTitle->GetColTarget();
            titleCol.w = 0.0f;
            pTitle->SetColTarget(titleCol);
            pTitle->SetCol(titleCol);
            m_vTitleTextAlpha[i] = 0.0f;

            if (pButtonA) {
                DirectX::XMFLOAT4 btnCol = pButtonA->GetColTarget();
                btnCol.w = 0.0f;
                pButtonA->SetColTarget(btnCol);
                pButtonA->SetCol(btnCol);
                m_vTitleButtonAAlpha[i] = 0.0f;
            }
        }
    }

    //ビームライトが出ていれば
    if (m_beamLightAppeared) {
        for (auto* beam : m_vpBeamLight) 
        {
            if (beam) {
                beam->Update();
                beam->Draw();
            }
        }
    }
}

//============================================================================
// シーン変更
//============================================================================
void CSceneResult::Change()
{
    // 全オブジェクトに死亡フラグを立てる
    CObjectManager::RefInstance().SetDeathAll();

    // エフェクトを全て停止
    CEffectManager::RefInstance().StopAll();

    //生存時間を破棄
    CPlayer::ClearAllSurvivalTimes();
    CEnemyPlayer::ClearAllCPUSurvivalTimes();

    CSoundManger::RefInstance().Stop("BGM_RESULT");
    CSoundManger::RefInstance().Play("Jump", false, 0.0f, 1.0f);

    // nullptr化（またはclear）でベクタ参照を無効化
    m_vpPlayerIcons.clear();
    m_vpPlayerLights.clear();
    m_vpNumbers.clear();
    m_vvPlayerNumbers.clear();

    //タイトル画面に遷移
    CSceneManager::RefInstance().ChangeScene(std::make_unique<CSceneTitle>());

}

//============================================================================
// 演出スキップ時処理
//============================================================================
void CSceneResult::ForceToFinished()
{
    //一旦既存の音声を全て止める
    CSoundManger::RefInstance().StopAll();

    CSoundManger::RefInstance().Play("Jump", false, 0.0f, 1.0f);

    // pResultTitleをゴール位置に移動
    if (m_resultTitleIdx >= 0 && m_resultTitleIdx < (int)m_vpNumbers.size()) {
        auto pHud = m_vpNumbers[m_resultTitleIdx];
        OBJ::Transform tr = pHud->GetTransform();
        tr.Pos.y = m_resultTitleTargetPos.y;
        pHud->SetTransform(tr);
        pHud->SetTransformTarget(tr);
    }

    // 各HUDのアルファ値を最大にする
    for (auto& pHud : m_vpPlayerLights)
        if (pHud) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = 1.0f; pHud->SetColTarget(col); pHud->SetCol(col); }
    for (auto& pHud : m_vpPlayerIcons)
        if (pHud) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = 1.0f; pHud->SetColTarget(col); pHud->SetCol(col); }
    for (auto& pHud : m_vpPlayerBattleTexts)
        if (pHud) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = 1.0f; pHud->SetColTarget(col); pHud->SetCol(col); }
    for (auto& pHud : m_vpPlayerRankImgs)
        if (pHud) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = 1.0f; pHud->SetColTarget(col); pHud->SetCol(col); }
    for (auto& vv : m_vvPlayerNumbers)
        for (auto& pHud : vv)
            if (pHud) { DirectX::XMFLOAT4 col = pHud->GetColTarget(); col.w = 1.0f; pHud->SetColTarget(col); pHud->SetCol(col); }
    for (auto idx : m_playerTextIdxs)
        if (idx >= 0 && idx < (int)m_vpPlayerTextImgs.size()) {
            auto pHud = m_vpPlayerTextImgs[idx];
            if (pHud) {
                DirectX::XMFLOAT4 col = pHud->GetColTarget();
                col.w = 1.0f;
                pHud->SetColTarget(col);
                pHud->SetCol(col);
            }
        }
    if (m_winTextIdx >= 0 && m_winTextIdx < (int)m_vpPlayerIcons.size())
    {
        auto pHud = m_vpPlayerIcons[m_winTextIdx];
        DirectX::XMFLOAT4 col = pHud->GetColTarget();
        col.w = 1.0f;
        pHud->SetColTarget(col);
        pHud->SetCol(col);
    }
    m_winTextAlpha = 255.0f;
    m_otherAlpha = 255.0f;
    m_playerTextScale = 1.0f;
    m_bgDarkRatio = 1.0f;

    // カメラ座標もリセット
    m_cameraHeight = 0.0f;
    CCamera* pCamera = CRenderer::RefInstance().GetCamera();
    if (pCamera) {
        DirectX::XMFLOAT3 pos = pCamera->GetPos();
        DirectX::XMFLOAT3 rot = pCamera->GetRot();
        pos.x = 0.0f;
        pos.y = 0.0f;
        pos.z = 0.0f;
        rot.x = 0.0f; rot.y = 0.0f; rot.z = 0.0f;
        pCamera->SetPos(pos);
        pCamera->SetRot(rot);
    }

    // kirakiraエフェクト生成
    if (!m_kirakiraEffectCreated && m_animPhase < ANIM_PHASE::PLAYER_WAIT)
    {
        if (m_winners.size() == 1)
        {
            CEffect::Create(CEffectManager::TAG_SPARKLE, { 21.0f, 1.0f, 0.0f }, {}, { 0.5f });
            CEffect::Create(CEffectManager::TAG_SPARKLE, { 12.6f, 2.8f, 0.0f }, {}, { 0.5f });
            CEffect::Create(CEffectManager::TAG_SPARKLE, { 6.5f, -0.8f, 0.0f }, {}, { 0.5f });
        }

        else if (m_winners.size() > 1)
        {
            CEffect::Create(CEffectManager::TAG_SPARKLE, { 17.8f, 1.0f, 0.0f }, {}, { 0.5f });
            CEffect::Create(CEffectManager::TAG_SPARKLE, { 12.6f, 3.4f, 0.0f }, {}, { 0.5f });
            CEffect::Create(CEffectManager::TAG_SPARKLE, { 10.5f, -0.8f, 0.0f }, {}, { 0.5f });
        }

        m_kirakiraEffectCreated = true;
    }

    if (m_animPhase < ANIM_PHASE::SHOW_OTHERS)
    {
        CSoundManger::RefInstance().Play("BGM_RESULT", false, 0.0f, 1.0f);
    }


    // ビームライト生成
    if (!m_beamLightAppeared && m_animPhase < ANIM_PHASE::PLAYER_TEXT_SCALEUP) {
        CSoundManger::RefInstance().Play("Drumroll_Finish", false, 0.0f, 1.0f);
        
        m_beamLightAppeared = true;
        m_vpBeamLight.clear();
        for (int i = 0; i < 3; ++i) {
            float time = 5.0f + i;
            DirectX::XMFLOAT2 pos = { 0.3f * (i == 0 ? 1 : i == 1 ? -1 : 0), 0.75f };
            auto pBeam = CObjectManager::CreateRaw<CBeamLight>(OBJ::TYPE::NONE, OBJ::LAYER::FRONT);
            pBeam->SetPos(pos);
            pBeam->SetTime(time);
            pBeam->SetEnableTime(true);
            m_vpBeamLight.push_back(pBeam);
        }
    }

    if (!m_WinnerModelAppeared)
    {
        const float baseX = -12.0f;
        const float spacing = 6.0f; // 勝者同士の間隔
        size_t winCount = m_winners.size();

        for (size_t k = 0; k < winCount; ++k)
        {
            int winnerIndex = m_winners[k];
            bool isHumanWinner = (winnerIndex < m_nHumanPlayerNum);
            std::string modelName;

            if (isHumanWinner) {
                modelName = "Player_" + std::to_string(winnerIndex + 1);
            }
            else {
                modelName = "Test";
            }

            // 横並び配置用にX座標をずらす
            float offsetX = baseX + spacing * ((int)k - (int)(winCount - 1) / 2.0f);

            auto model = CObjectManager::CreateShare<CPhysicsModel>(
                [modelName, offsetX](CPhysicsModel* p) -> bool
                {
                    p->SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey(modelName));
                    OBJ::Transform tf;
                    tf.Pos = { offsetX, 0.0f, 0.0f };
                    tf.Rot = { 0.0f, 0.0f, 0.0f, 1.0f };
                    p->SetTransform(tf);
                    p->FactoryCollider(1.0f, 1.0f, 1.0f);
                    return true;
                },
                OBJ::TYPE::NONE, OBJ::LAYER::FRONT
                    );

            if (k == 0) m_spTestModel = model;
        }

        m_WinnerModelAppeared = true;
    }

    // アニメフェーズをFINISHEDにする
    m_animPhase = ANIM_PHASE::FINISHED;
}