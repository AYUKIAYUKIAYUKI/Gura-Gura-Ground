//============================================================================
// 
// HUD：カウント、ヘッダーファイル [hud.count.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.hud.h"
#include "API.window.h"

//****************************************************
// HUD：カウントクラスの定義
//****************************************************
class CHudCount : public CHud
{
	//****************************************************
	// 静的メンバ定数の定義
	//****************************************************

	// 画面中央の座標
	static constexpr DirectX::XMFLOAT3 POS_CENTER = { CWindow::FSCREEN_WIDTH * 0.5f, CWindow::FSCREEN_HEIGHT * 0.5f, 0.0f };
	
	// 画面中央のトランスフォーム
	static const OBJ::Transform TRANSFORM_CENTER_DISP; // 表示
	static const OBJ::Transform TRANSFORM_CENTER_OFF;  // 非表示

public:

	//****************************************************
	// special function
	//****************************************************
	CHudCount(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CHudCount() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// カウントのインデックス
	void SetHudCountIdx(unsigned char wIdx);

	// 現在のカウントを反映
	void SetNowCount(unsigned char wIdx);

private:

	//****************************************************
	// data
	//****************************************************
	unsigned char m_wHudCountIdx; // カウントのインデックス
};