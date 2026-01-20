//============================================================================
// 
// シンボル [symbol.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "symbol.h"
#include "API.renderer.h"
#include "API.texture.manager.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSymbol::CSymbol(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CRect3D(Type, Layer)
	, m_fSymbolOffsetY(2.0f)
{
	// パイプラインのプリセット
	SetVertexShader(CVertexShaderManager::RefInstance().RefRegistry().BindAtKey("Vertex.3D"));
	SetInputLayout(CInputLayoutManager::RefInstance().RefInputLayout(CInputLayoutManager::VertexType::VERTEX_3D));
	SetPixelShader(CPixelShaderManager::RefInstance().RefRegistry().BindAtKey("Vertex.3D"));

	/* 仮テクスチャのバインド */
	SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Test"));

	// トランスフォームの設定
	SetTransform({ { 2.0f, 2.0f, 0.0f }, { DirectX::XM_PI, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } });
}

//============================================================================
// デストラクタ
//============================================================================
CSymbol::~CSymbol()
{}

//============================================================================
// 更新処理
//============================================================================
void CSymbol::Update()
{
	// 四角形(3D)の更新処理
	CRect3D::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CSymbol::Draw()
{
	// 四角形(3D)の描画処理
	CRect3D::Draw();
}

//============================================================================
// シンボルのインデックス設定
//============================================================================
void CSymbol::SetSymbolIdx(unsigned char wIdx)
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

	case 3:
		SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("P4"));
		break;

	default:
		SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("CP"));
		break;
	}
}