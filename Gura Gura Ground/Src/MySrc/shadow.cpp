//============================================================================
// 
// 影 [shadow.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "shadow.h"
#include "API.renderer.h"
#include "API.texture.manager.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CShadow::CShadow(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CRect3D(Type, Layer)
{
	// パイプラインのプリセット
	SetVertexShader(CVertexShaderManager::RefInstance().RefRegistry().BindAtKey("Vertex.3D"));
	SetInputLayout(CInputLayoutManager::RefInstance().RefInputLayout(CInputLayoutManager::VertexType::VERTEX_3D));
	SetPixelShader(CPixelShaderManager::RefInstance().RefRegistry().BindAtKey("Vertex.3D"));

	// テクスチャのバインド
	SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Shadow"));

	// ブレンドタイプの設定：減算合成
	SetBlendType(1);

	// トランスフォームの設定
	SetTransform({ { 10.0f, 10.0f, 0.0f }, { DirectX::XM_PI * -0.5f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } });
}

//============================================================================
// デストラクタ
//============================================================================
CShadow::~CShadow()
{}

//============================================================================
// 更新処理
//============================================================================
void CShadow::Update()
{
	// 四角形(3D)の更新処理
	CRect3D::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CShadow::Draw()
{
	// 四角形(3D)の描画処理
	CRect3D::Draw();
}