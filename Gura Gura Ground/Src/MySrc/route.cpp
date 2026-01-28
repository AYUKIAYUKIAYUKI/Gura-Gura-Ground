//============================================================================
// 
// 進路 [route.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "route.h"
#include "obstacle.h"
#include "API.texture.manager.h"

// 追従対象の取得のため
#include "ball.h"
#include "bar.h"
#include "barrel.h"
#include "birdstrike.h"
#include "pendulum.h"
#include "API.rigidbody.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CRoute::CRoute(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CRect3D(Type, Layer)
	, m_nDirection(0)
	, m_fBright(0.0f)
	, m_fBrightFactor(FACTOR_BRIFHT)
{
	// パイプラインのプリセット
	SetVertexShader(CVertexShaderManager::RefInstance().RefRegistry().BindAtKey("Vertex.3D"));
	SetInputLayout(CInputLayoutManager::RefInstance().RefInputLayout(CInputLayoutManager::VertexType::VERTEX_3D));
	SetPixelShader(CPixelShaderManager::RefInstance().RefRegistry().BindAtKey("Vertex.3D"));

	// テクスチャのバインド
	SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("Route"));

	// ブレンドタイプの設定：加算
	SetBlendType(CRenderer::BlendType::ADD);
}

//============================================================================
// デストラクタ
//============================================================================
CRoute::~CRoute()
{}

//============================================================================
// 更新処理
//============================================================================
void CRoute::Update()
{
	// 明度の更新
	UpdateBright();

	// 追従対象の障害物が有効なら
	if (std::shared_ptr<CObstacle> spTarget = m_wpTrackTarget.lock())
	{
		// 追従対象のトランスフォームを取得
		const OBJ::Transform Transform = spTarget->GetTransform();

		/* 地面の大きさ */
		const float fFieldSize = 30.0f;

		/* 地面の位置 */
		const float fFieldY = 5.0f + 1.08f;

		// 進行方向の初期化
		DirectX::XMFLOAT3 Direction = useful::VEC3_ZERO_INIT;

		// 特定の型の障害物に応じてトランスフォームを調整
		/* とんでもない処理だ… */
		if (const std::shared_ptr<CBall> spBall = std::dynamic_pointer_cast<CBall>(spTarget))
		{
			// 進行方向を取得
			Direction = spBall->GetDirection();
		}
		else if (const std::shared_ptr<CBar> spBar = std::dynamic_pointer_cast<CBar>(spTarget))
		{
			// 進行方向を取得
			Direction = spBar->GetDirection();
		}
		else if (const std::shared_ptr<CBarrel> spBarrel = std::dynamic_pointer_cast<CBarrel>(spTarget))
		{
			// 進行方向を取得
			Direction = spBarrel->GetDirection();
		}
		else if (const std::shared_ptr<CPendulum> spPendulum = std::dynamic_pointer_cast<CPendulum>(spTarget))
		{
			// 進行方向を取得
			Direction = spPendulum->GetDirection();
		}

		// 進行方向から、X方向移動かZ方向移動かを判定しトランスフォームを調整
		if (std::abs(Direction.x) > std::abs(Direction.z))
		{
			// X方向移動
			SetTransform({
				{ fFieldSize, Transform.Size.y * 2.0f, 0.0f },
				{ DirectX::XM_PI * -0.5f, DirectX::XM_PI, 0.0f, 1.0f },
				{ 0.0f, fFieldY, Transform.Pos.z } });
		}
		else
		{
			// Z方向移動
			SetTransform({
				{ fFieldSize, Transform.Size.y * 2.0f, 0.0f },
				{ DirectX::XM_PI * -0.5f, DirectX::XM_PI * 0.5f, 0.0f, 1.0f },
				{ Transform.Pos.x, fFieldY, 0.0f } });
		}

		/* やばすぎる */
		if (const std::shared_ptr<CBirdStrike> spBirdStrike = std::dynamic_pointer_cast<CBirdStrike>(spTarget))
		{
			// 進行方向を取得
			Direction = spBirdStrike->GetDirection();

			SetTransform({
				{ fFieldSize * 1.5f, Transform.Size.y * 2.0f, 0.0f },
				{ DirectX::XM_PI * -0.5f, DirectX::XM_PI + atan2f(-Direction.z, Direction.x), 0.0f, 1.0f},
				{ Transform.Pos.x, fFieldY, Transform.Pos.z } });
		}

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
void CRoute::Draw()
{
	// 四角形(3D)の描画処理
	CRect3D::Draw();
}

//============================================================================
// 追従対象の障害物の設定
//============================================================================
void CRoute::SetTrackTarget(const std::shared_ptr<CObstacle>& spTarget)
{
	// 弱参照を作成しておく
	m_wpTrackTarget = spTarget;
}

//============================================================================
// 明度の更新
//============================================================================
void CRoute::UpdateBright()
{
	// 明度を変更
	m_fBright += m_fBrightFactor;

	// 明度が最小・最大を設定を越したら変化量を反転
	if (m_fBright < 0.0f || m_fBright > 1.0f)
	{
		m_fBrightFactor *= -1.0f;
	}
}