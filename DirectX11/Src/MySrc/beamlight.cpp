//============================================================================
//
// ビームライト [beamlight.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "beamlight.h"
#include "API.renderer.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CBeamLight::CBeamLight(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CFullScreen2D(Type, Layer)
	, m_Pos(useful::VEC2_ZERO_INIT)
	, m_fTime(0.0f)
	, m_bEnableTime(false)
{
	// 定数バッファを生成
	CConstantBufferManager::CreateConstantbuffer(
		sizeof(CB_BeamLight),
		m_cpConstantBuffer.GetAddressOf());

	// そのまま基底へバインド
	CFullScreen2D::SetConstantBuffer(m_cpConstantBuffer);

	// パイプライン設定のプリセット
	SetVertexShader(CVertexShaderManager::RefInstance().RefRegistry().BindAtKey("FullScreen.2D"));
	SetPixelShader(CPixelShaderManager::RefInstance().RefRegistry().BindAtKey("Light.Triple"));
}

//============================================================================
// デストラクタ
//============================================================================
CBeamLight::~CBeamLight()
{}

//============================================================================
// 更新処理
//============================================================================
void CBeamLight::Update()
{
	// 定数バッファの更新
	UpdateConstantBuffer();

	CFullScreen2D::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CBeamLight::Draw()
{
	CFullScreen2D::Draw();
}

//============================================================================
// 定数バッファの更新
//============================================================================
void CBeamLight::UpdateConstantBuffer()
{	
	if (m_bEnableTime)
	{
		// 時間の更新
		m_fTime += 1.0f / 60.0f;
	}

	// コンテキストの取得
	ID3D11DeviceContext* pContext = CRenderer::RefInstance().GetContext();

	// GPUに送る定数バッファデータ
	CB_BeamLight cbBeamLight = { m_Pos, m_fTime, 0.0f };

	// 定数バッファの更新
	CConstantBufferManager::UpdateConstantBuffer(pContext, m_cpConstantBuffer.Get(), cbBeamLight);
}