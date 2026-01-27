//============================================================================
// 
// HUD：カウント [hud.count.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "hud.count.h"
#include "API.texture.manager.h"

//****************************************************
// 静的メンバ定数の定義
//****************************************************

// 画面中央のトランスフォーム：表示
const OBJ::Transform CHudCount::TRANSFORM_CENTER_DISP =
{
	DirectX::XMFLOAT3(800.0f, 800.0f, 0.0f),
	DirectX::XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f),
	CHudCount::POS_CENTER
};

// 画面中央のトランスフォーム：非表示
const OBJ::Transform CHudCount::TRANSFORM_CENTER_OFF =
{
	useful::VEC3_ZERO_INIT,
	DirectX::XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f),
	CHudCount::POS_CENTER
};

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CHudCount::CHudCount(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CHud(Type, Layer)
	, m_wHudCountIdx(0)
{
	SetTransform(TRANSFORM_CENTER_OFF);
	SetTransformTarget(TRANSFORM_CENTER_OFF);
}

//============================================================================
// デストラクタ
//============================================================================
CHudCount::~CHudCount()
{}

//============================================================================
// 更新処理
//============================================================================
void CHudCount::Update()
{
	// HUDの更新処理
	CHud::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CHudCount::Draw()
{
	// HUDの更新処理
	CHud::Draw();
}

//============================================================================
// シンボルのインデックス設定
//============================================================================
void CHudCount::SetHudCountIdx(unsigned char wIdx)
{
	// テクスチャのバインド
	switch (wIdx)
	{
	case 0:
		SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("P1"));
		break;

	case 1:
		SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("P2"));
		break;

	case 2:
		SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("P3"));
		break;

	default:
		SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("CP"));
		break;
	}

	m_wHudCountIdx = wIdx;
}

//============================================================================
// カウントのインデックス設定
//============================================================================
void CHudCount::SetNowCount(unsigned char wIdx)
{
	if (m_wHudCountIdx == wIdx)
	{
		// ランダムに微小な揺れを加える
		OBJ::Transform TF = GetTransform();
		TF.Pos =
		{
			TF.Pos.x + useful::GetRandomValue<float>() * 0.01f,
			TF.Pos.y + useful::GetRandomValue<float>() * 0.01f,
			0.0f
		};
		SetTransform(TF);

		// 該当インデックスの場合、中央にて大きく表示
		SetTransformTarget(TRANSFORM_CENTER_DISP);
	}
	else
	{
		// 該当インデックスでない場合、中央にて通常表示
		SetTransformTarget(TRANSFORM_CENTER_OFF);
	}
}