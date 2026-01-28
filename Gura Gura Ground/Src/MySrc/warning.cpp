//============================================================================
// 
// 警告 [warning.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "warning.h"
#include "obstacle.h"
#include "API.texture.manager.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CWarning::CWarning(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CRect3D(Type, Layer)
	, m_fRotY(0.0f)
	, m_fBright(0.0f)
	, m_fBrightFactor(FACTOR_BRIFHT)
{
	// パイプラインのプリセット
	SetVertexShader(CVertexShaderManager::RefInstance().RefRegistry().BindAtKey("Vertex.3D"));
	SetInputLayout(CInputLayoutManager::RefInstance().RefInputLayout(CInputLayoutManager::VertexType::VERTEX_3D));
	SetPixelShader(CPixelShaderManager::RefInstance().RefRegistry().BindAtKey("Vertex.3D"));

	// テクスチャのバインド
	SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Warning"));

	// ブレンドタイプの設定：加算
	SetBlendType(CRenderer::BlendType::ADD);
}

//============================================================================
// デストラクタ
//============================================================================
CWarning::~CWarning()
{}

//============================================================================
// 更新処理
//============================================================================
void CWarning::Update()
{
	// 回転量を増加
	m_fRotY += SPEED_ROTATE;

	// 明度の更新
	UpdateBright();

	// 追従対象の障害物が有効なら
	if (std::shared_ptr<CObstacle> spTarget = m_wpTrackTarget.lock())
	{
		// 追従対象のトランスフォームを取得
		const OBJ::Transform TF = spTarget->GetTransform();

		/* 地面の位置 */
		const float fFieldY = 5.0f + 1.08f;

		// 位置に応じてトランスフォームを調整
		SetTransform({
			{ (TF.Size.x * 10.0f), (TF.Size.y * 10.0f), 0.0f },
			{ DirectX::XM_PI * -0.5f, m_fRotY, 0.0f, 1.0f },
			{ TF.Pos.x, fFieldY, TF.Pos.z } });

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
void CWarning::Draw()
{
	// 四角形(3D)の描画処理
	CRect3D::Draw();
}

//============================================================================
// 追従対象の障害物の設定
//============================================================================
void CWarning::SetTrackTarget(const std::shared_ptr<CObstacle>& spTarget)
{
	// 弱参照を作成しておく
	m_wpTrackTarget = spTarget;
}

//============================================================================
// 明度の更新
//============================================================================
void CWarning::UpdateBright()
{
	// 明度を変更
	m_fBright += m_fBrightFactor;

	// 明度が最小・最大を設定を越したら変化量を反転
	if (m_fBright < 0.0f || m_fBright > 1.0f)
	{
		m_fBrightFactor *= -1.0f;
	}
}