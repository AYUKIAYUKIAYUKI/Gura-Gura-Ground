//============================================================================
// 
// アーチ [arch.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "arch.h"
#include "boomerang.h"
#include "API.texture.manager.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CArch::CArch(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CRect3D(Type, Layer)
	, m_fBright(0.0f)
	, m_fBrightFactor(FACTOR_BRIFHT)
{
	// パイプラインのプリセット
	SetVertexShader(CVertexShaderManager::RefInstance().RefRegistry().BindAtKey("Vertex.3D"));
	SetInputLayout(CInputLayoutManager::RefInstance().RefInputLayout(CInputLayoutManager::VertexType::VERTEX_3D));
	SetPixelShader(CPixelShaderManager::RefInstance().RefRegistry().BindAtKey("Vertex.3D"));

	// テクスチャのバインド
	SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Arch"));

	// ブレンドタイプの設定：加算
	SetBlendType(CRenderer::BlendType::ADD);
}

//============================================================================
// デストラクタ
//============================================================================
CArch::~CArch()
{}

//============================================================================
// 更新処理
//============================================================================
void CArch::Update()
{
	// 明度の更新
	UpdateBright();

	// 追従対象の障害物が有効なら
	if (std::shared_ptr<CBoomerang> spTarget = m_wpTrackTarget.lock())
	{
		// 位置に応じて色を調整
		SetCol({ 1.0f, 0.0f, 0.0f, m_fBright });
	}
	else
	{
		SetDeath();
	}

	// 四角形(3D)の更新処理
	CRect3D::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CArch::Draw()
{
	// 四角形(3D)の描画処理
	CRect3D::Draw();
}

//============================================================================
// 追従対象の障害物の設定
//============================================================================
void CArch::SetTrackTarget(const std::shared_ptr<CBoomerang>& spTarget)
{
	// 弱参照を作成しておく
	m_wpTrackTarget = spTarget;

	// 回転の半径を取得
	const float fRadius = spTarget->GetRadius();

	// 回転の中心点を取得
	const DirectX::XMFLOAT3& Center = spTarget->GetCenter();

	// 弧の開始角度を取得
	const float fAngleY = spTarget->GetStartAngle();

	/* 地面の位置 */
	const float fFieldY = 5.0f + 1.08f;

	// トランスフォームを設定
	SetTransform({
		{ fRadius * 2.0f, fRadius * 2.0f, 0.0f },
		{ DirectX::XM_PI * -0.5f, -fAngleY, 0.0f, 1.0f },
		{ Center.x, fFieldY, Center.z } });
}

//============================================================================
// 明度の更新
//============================================================================
void CArch::UpdateBright()
{
	// 明度を変更
	m_fBright += m_fBrightFactor;

	// 明度が最小・最大を設定を越したら変化量を反転
	if (m_fBright < 0.0f || m_fBright > 1.0f)
	{
		m_fBrightFactor *= -1.0f;
	}
}