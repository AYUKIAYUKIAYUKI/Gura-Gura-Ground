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
	, m_BaseCameraDistance(0.0f)
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

	// 基準の距離
	m_BaseCameraDistance = 30.0f;

	//最大の距離
	m_MaxCameraDistance = m_Camera->GetDistance();

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
	if (m_Players.empty())
	{
		return;
	}

	// カメラの注視点位置設定
	CalculatePlayersCenter();

	// カメラの変更位置を設定
	m_Camera->SetPosTarget(m_PlayersCenterPos);
}

//============================================================================
// プレイヤーの中心位置を計算
//============================================================================
void CCameraController::CalculatePlayersCenter()
{
	using namespace useful;

	DirectX::XMFLOAT3 MinPlayersPos = { 0.0f,0.0f, 0.0f };	// 最小位置
	DirectX::XMFLOAT3 MaxPlayersPos = { 0.0f,0.0f, 0.0f };	// 最大位置
	DirectX::XMFLOAT3 CenterPos = { 0.0f,0.0f, 0.0f };		// 中央位置
	DirectX::XMFLOAT3 Size = { 0.0f,0.0f, 0.0f };			// 幅

	// プレイヤー全てで最小最大の位置を取得
	GetPlayersBounds(MinPlayersPos, MaxPlayersPos);

	// 中心位置を求める
	CenterPos = (MaxPlayersPos + MinPlayersPos) * 0.5f;

	// プレイヤーの広がりを計算
	float SpreadX = MaxPlayersPos.x - MinPlayersPos.x;

	// 高さの設定
	CenterPos.y = m_Camera->GetPos().y;

	// 中心位置に設定
	m_PlayersCenterPos = CenterPos;

	// 距離を計算
	float Distance = (m_BaseCameraDistance + (SpreadX * 0.7f));

	// 遠すぎ
	if (Distance > m_MaxCameraDistance)
	{
		Distance = m_MaxCameraDistance;
	}

	// 距離設定
	m_Camera->SetDistanceTarget(Distance);
}

//============================================================================
// プレイヤー全てで最小最大の位置を取得
//============================================================================
void CCameraController::GetPlayersBounds(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max)
{
	using namespace useful;

	DirectX::XMFLOAT3 MinPlayersPos = { 0.0f,0.0f, 0.0f };	// 最小位置
	DirectX::XMFLOAT3 MaxPlayersPos = { 0.0f,0.0f, 0.0f };	// 最大位置

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

	// 位置を設定
	min = MinPlayersPos;
	max = MaxPlayersPos;
}