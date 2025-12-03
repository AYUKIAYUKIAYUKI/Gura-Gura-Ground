//============================================================================
// 
// カメラの移動制御 [cameracontroller.cpp]
// Author : 後藤優輝
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "cameracontroller.h"
#include "API.renderer.h"
#include "API.camera.h"
#include "player.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CCameraController::CCameraController()
	: m_Camera(nullptr)
	, m_CenterPos({0.0f,3.0f,0.0f})
{
	
}

//============================================================================
// デストラクタ
//============================================================================
CCameraController::~CCameraController()
{
	
}

//============================================================================
// 初期設定
//============================================================================
bool CCameraController::Initialize()
{
	// 初期化
	m_Players.clear();

	// カメラの情報取得
	m_Camera = nullptr;
	m_Camera = CRenderer::RefInstance().GetCamera();

	return true;
}

//============================================================================
// 終了処理
//============================================================================
void CCameraController::Finalize()
{}

//============================================================================
// 更新処理
//============================================================================
void CCameraController::Update()
{
	// カメラ移動
	CameraMove();
}

//============================================================================
// プレイヤーの登録
//============================================================================
void CCameraController::Regist(CPlayer* player)
{
	m_Players.push_back(player);
}

//============================================================================
// プレイヤーの削除
//============================================================================
void CCameraController::UnRegist(CPlayer* player)
{
	for (auto ite = m_Players.begin(); ite != m_Players.end(); ite++)
	{
		if (*ite == player)
		{// 同じとき

			// 削除
			ite = m_Players.erase(ite);
			break;
		}
	}
}

//============================================================================
// カメラ移動
//============================================================================
void CCameraController::CameraMove()
{
	using namespace useful;

	// ターゲットの位置設定
	DirectX::XMFLOAT3 TargetPos = DetermineCameraPosition();
	
	m_Camera->SetPosTarget(TargetPos);
}

//============================================================================
// カメラの位置を指定
//============================================================================
DirectX::XMFLOAT3 CCameraController::DetermineCameraPosition()
{
	bool isMovingRight = true;
	bool isMovingLeft = true;
	DirectX::XMFLOAT3 CameraPos;// カメラの位置

	for (auto ite : m_Players)
	{
		// プレイヤーの位置取得
		DirectX::XMFLOAT3 PlayerPos = ite->GetTransform().Pos;

		if (PlayerPos.x < m_CenterPos.x)
		{
			isMovingRight = false;
		}

		if (PlayerPos.x > m_CenterPos.x)
		{
			isMovingLeft = false;
		}
	}

	if (isMovingRight)
	{
		CameraPos = { 10.0f,m_Camera->GetPos().y ,m_Camera->GetPos().z };
	}
	else if(isMovingLeft)
	{
		CameraPos = { -10.0f,m_Camera->GetPos().y ,m_Camera->GetPos().z };
	}
	else if (!isMovingLeft&&!isMovingRight)
	{
		CameraPos = { 0.0f,m_Camera->GetPos().y ,m_Camera->GetPos().z };
	}


	return CameraPos;
}