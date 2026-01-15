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
	CHud* m_pBackground;
	std::vector<CHud*> m_vpNumbers; // 0-9画像HUD
	int m_nResultValue;
	float m_fGameTime;
	std::vector<float> m_playerSurvivalTimes;
	std::vector<std::vector<CHud*>> m_vvPlayerNumbers;
};