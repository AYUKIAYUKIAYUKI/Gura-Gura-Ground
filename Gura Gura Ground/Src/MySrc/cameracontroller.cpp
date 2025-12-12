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
#include "API.object.manager.h"
#include "obstacle.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CCameraController::CCameraController()
	: m_Camera(nullptr)
	, m_CameraTargetPos({0.0f,0.0f,0.0f})
	, m_BaseCameraDistance(0.0f)
	, m_IsMovingGimmickActive(false)
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
	m_Obstacles.clear();

	// カメラの情報取得
	m_Camera = nullptr;
	m_Camera = CRenderer::RefInstance().GetCamera();

	// ギミック出現してない
	m_IsMovingGimmickActive = false;

	// 基準の距離
	m_BaseCameraDistance = 30.0f;

	//最大の距離
	m_MaxCameraDistance = m_Camera->GetDistance();

	// カメラの最初の位置
	m_FirstCameraPos = m_Camera->GetPos();

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
	// ギミックを取得
	GetObstacles();

	// 移動ギミックがあるか判定
	HasMovingGimmick();

	if (!m_IsMovingGimmickActive)
	{
		// カメラの注視点位置設定
		CalculateCenter();
	}

	// カメラの変更位置を設定
	m_Camera->SetPosTarget(m_CameraTargetPos);
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
// 中心位置を計算
//============================================================================
void CCameraController::CalculateCenter()
{
	using namespace useful;

	DirectX::XMFLOAT3 MinPlayersPos = { 0.0f,0.0f, 0.0f };	// 最小位置
	DirectX::XMFLOAT3 MaxPlayersPos = { 0.0f,0.0f, 0.0f };	// 最大位置
	DirectX::XMFLOAT3 CenterPos = { 0.0f,0.0f, 0.0f };		// 中央位置
	DirectX::XMFLOAT3 Size = { 0.0f,0.0f, 0.0f };			// 幅

	// プレイヤーと定点のギミックから最小最大位置取得
	GetPlayersAndObstaclesBounds(MinPlayersPos, MaxPlayersPos);

	// 中心位置を求める
	CenterPos = (MaxPlayersPos + MinPlayersPos) * 0.5f;

	// 高さの設定
	CenterPos.y = m_Camera->GetPos().y;

	// 中心位置に設定
	m_CameraTargetPos = CenterPos;

	//// プレイヤーの広がりを計算
	//float SpreadX = MaxPlayersPos.x - MinPlayersPos.x;

	//// 距離を計算
	//float Distance = (m_BaseCameraDistance + (SpreadX * 0.7f));

	//// 遠すぎ
	//if (Distance > m_MaxCameraDistance)
	//{
	//	Distance = m_MaxCameraDistance;
	//}

	//// 距離設定
	//m_Camera->SetDistanceTarget(Distance);
}

//============================================================================
// プレイヤーと定点のギミックから最小最大位置取得
//============================================================================
void CCameraController::GetPlayersAndObstaclesBounds(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max)
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
		MaxPlayersPos.x = max(MaxPlayersPos.x, ite->GetTransform().Pos.x);
		MinPlayersPos.x = min(MinPlayersPos.x, ite->GetTransform().Pos.x);
		
		// Z座標の最小最大
		MaxPlayersPos.z = max(MaxPlayersPos.z, ite->GetTransform().Pos.z);
		MinPlayersPos.z = min(MinPlayersPos.z, ite->GetTransform().Pos.z);

		Count++;
	}

	// ギミックで最小と最大の位置を取得
	for (auto ite : m_Obstacles)
	{
		if (ite->GetObsType() != Obstacle::OBSTACLE_TYPE::STATIONARY)
		{
			continue;
		}

		// X座標の最小最大
		MaxPlayersPos.x = max(MaxPlayersPos.x, ite->GetTransform().Pos.x);
		MinPlayersPos.x = min(MinPlayersPos.x, ite->GetTransform().Pos.x);

		// Z座標の最小最大
		MaxPlayersPos.z = max(MaxPlayersPos.z, ite->GetTransform().Pos.z);
		MinPlayersPos.z = min(MinPlayersPos.z, ite->GetTransform().Pos.z);
	}

	// 位置を設定
	min = MinPlayersPos;
	max = MaxPlayersPos;
}

//============================================================================
// ギミックを取得
//============================================================================
void CCameraController::GetObstacles()
{
	// 障害物リスト取得
	auto List = CObjectManager::RefInstance().RefObjList(OBJ::TYPE::OBSTACLE);

	for (auto ite : List)
	{
		CObstacle* Obstacle = dynamic_cast<CObstacle*>(ite);

		m_Obstacles.push_back(Obstacle);
	}
}

//============================================================================
// ギミックがあるか判定
//============================================================================
void CCameraController::HasMovingGimmick()
{
	// 判定を戻す
	m_IsMovingGimmickActive = false;

	for (auto ite : m_Obstacles)
	{
		if (ite->GetObsType() == Obstacle::OBSTACLE_TYPE::MOVING)
		{
			m_IsMovingGimmickActive = true;
		}
	}

	// 動くギミックない
	if (m_IsMovingGimmickActive)
	{
		// 最初の位置に戻る
		m_CameraTargetPos = m_FirstCameraPos;
	}
}