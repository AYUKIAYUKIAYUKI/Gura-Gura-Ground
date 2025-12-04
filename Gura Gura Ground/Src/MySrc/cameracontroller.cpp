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
	, m_PlayersCenterPos({0.0f,0.0f,0.0f})
	, m_BaseHeight(0.0f)
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

	// 最初の高さ
	m_BaseHeight = m_Camera->GetPos().y;

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
	// カメラの注視点位置設定
	CalculatePlayersCenter();

	m_Camera->SetPosTarget(m_PlayersCenterPos);
}

//============================================================================
// プレイヤーの中心位置を計算
//============================================================================
void CCameraController::CalculatePlayersCenter()
{
	if (m_Players.empty())
	{
		return;
	}

	using namespace useful;

	DirectX::XMFLOAT3 MinPlayersPos = { 0.0f,0.0f, 0.0f };	// 最小位置
	DirectX::XMFLOAT3 MaxPlayersPos = { 0.0f,0.0f, 0.0f };	// 最大位置
	DirectX::XMFLOAT3 CenterPos = { 0.0f,0.0f, 0.0f };		// 中央位置
	DirectX::XMFLOAT3 Size = { 0.0f,0.0f, 0.0f };			// 幅の大きさ

	int Count = 0;

	// プレイヤー全体で最小と最大の位置を取得
	for (auto ite : m_Players)
	{
		if (Count == 0)
		{
			MinPlayersPos = ite->GetTransform().Pos;
			MaxPlayersPos = ite->GetTransform().Pos;
		}

		// X座標の最小最大
		if (ite->GetTransform().Pos.x <= MinPlayersPos.x)
		{
			MinPlayersPos.x = ite->GetTransform().Pos.x;
		}
		if (MaxPlayersPos.x <= ite->GetTransform().Pos.x)
		{
			MaxPlayersPos.x = ite->GetTransform().Pos.x;
		}

		// Z座標の最小最大
		if (ite->GetTransform().Pos.z <= MinPlayersPos.z)
		{
			MinPlayersPos.z = ite->GetTransform().Pos.z;
		}
		if (MaxPlayersPos.z <= ite->GetTransform().Pos.z)
		{
			MaxPlayersPos.z = ite->GetTransform().Pos.z;
		}

		Count++;
	}

	// 最小最大位置の幅を求める
	Size = MaxPlayersPos + MinPlayersPos;

	// 中心位置を求める
	CenterPos = Size / 2;

	// 高さを求める
	CenterPos.y = m_Camera->GetPos().y;

	// 中心位置に設定
	m_PlayersCenterPos = CenterPos;
}