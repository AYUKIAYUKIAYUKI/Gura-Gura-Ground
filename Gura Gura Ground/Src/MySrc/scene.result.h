//============================================================================
// 
// リザルトシーンヘッダファイル [scene.result.h]
// Author : Copilot
// 
//============================================================================

#pragma once

#include "API.scene.h"

class CHud;

class CSceneResult final : public CScene
{
public:
	CSceneResult(const std::vector<float>& playerSurvivalTimes);
	~CSceneResult() override;

	void Update() override;
	void Change() override;

private:
	enum class ANIM_PHASE {
		TITLE_MOVE,				// 結果発表タイトル
		TITLE_WAIT,				// アニメーション待機
		WIN_TEXT_FADEIN,        // WinTextのα加算
		WIN_WAIT,               // アニメーション待機
		PLAYER_TEXT_SCALEUP,    // 勝者テキスト拡大
		PLAYER_WAIT,            // アニメーション待機
		SHOW_OTHERS,            // BG以外HUDのα加算
		FINISHED
	};
	ANIM_PHASE m_animPhase = ANIM_PHASE::WIN_TEXT_FADEIN;
	float m_animTimer = 0.0f;
	float m_winTextAlpha = 0.0f;
	int   m_winTextAlphaInt = 0;
	float m_playerTextScale = 0.0f;
	float m_otherAlpha = 0.0f;
	int   m_otherAlphaInt = 0;

	// 各種類のHUDへのindex/memo
	int m_winTextIdx = -1;
	std::vector<int> m_playerTextIdxs;
	std::vector<int> m_otherHudIdxs;

	CHud* m_pBackground;
	std::vector<CHud*> m_vpNumbers;
	int m_nResultValue;
	float m_fGameTime;
	int m_resultTitleIdx = -1;
	DirectX::XMFLOAT3 m_resultTitleTargetPos;
	std::vector<float> m_playerSurvivalTimes;
	std::vector<std::vector<CHud*>> m_vvPlayerNumbers;
	std::vector<CHud*> m_vpPlayerLights;
	std::vector<CHud*> m_vpPlayerBattleTexts;
	std::vector<CHud*> m_vpPlayerIcons;
	std::vector<CHud*> m_vpPlayerRankImgs;
	std::vector<CHud*> m_vpPlayerTextImgs;
};