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
#include "API.texture.manager.h"
#include "API.physics.model.h"

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
	SetBlendType(CRenderer::BlendType::SUB);
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
#if 0 
	useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
	static float fMargin = 1.08f;
	ImGui::DragFloat("Margin", &fMargin, 0.01f);
	ImGui::End();
#endif // 0

	// 追従対象のオブジェクトが有効なら
	if (std::shared_ptr<CObject> spTarget = m_wpTrackTarget.lock())
	{
		// 追従対象が物理モデルなら
		if (std::shared_ptr<CPhysicsModel> spModel = std::dynamic_pointer_cast<CPhysicsModel>(spTarget))
		{
			// 追従対象のトランスフォームを取得
			const OBJ::Transform TF = spModel->GetTransform();

			/* 地面の位置 */
			const float fFieldY = 5.0f + 1.08f;
		
			// 高低差の調整値
			const float fAdjust = (TF.Pos.y - fFieldY);
	
			// 位置に応じてトランスフォームを調整
			SetTransform({
				{ (TF.Size.x * 5.0f) + fAdjust * 0.5f , (TF.Size.y * 5.0f) + fAdjust * 0.5f , 0.0f },
				{ DirectX::XM_PI * -0.5f, 0.0f, 0.0f, 1.0f },
				{ TF.Pos.x, fFieldY, TF.Pos.z } });

			// 位置に応じて色を調整
			SetCol({ 1.0f, 1.0f, 1.0f, 1.0f - fAdjust * 0.2f });
		}
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
void CShadow::Draw()
{
	// 四角形(3D)の描画処理
	CRect3D::Draw();
}

//============================================================================
// 追従対象の物理モデルの設定
//============================================================================
void CShadow::SetTrackTarget(const std::shared_ptr<CObject>& spTarget)
{
	// 弱参照を作成しておく
	m_wpTrackTarget = spTarget;
}