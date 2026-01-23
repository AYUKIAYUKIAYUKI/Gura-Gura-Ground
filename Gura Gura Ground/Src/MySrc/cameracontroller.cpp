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
#include "field.h"

/* 追加 */
#include "enemy1.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CCameraController::CCameraController()
	: m_Camera(nullptr)
	, m_CameraTargetPos({0.0f,0.0f,0.0f})
	, m_BaseCameraDistance(0.0f)
	, m_IsMovingGimmickActive(false)
	, m_IsPerimeterGimmickActive(false)
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
	using namespace useful;

	// 初期化
	m_Players.clear();
	m_Obstacles.clear();

	// カメラの情報取得
	m_Camera = nullptr;
	m_Camera = CRenderer::RefInstance().GetCamera();

	// ギミック出現してない
	m_IsMovingGimmickActive = false;
	m_IsPerimeterGimmickActive = false;

	// 基準の距離
	m_BaseCameraDistance = 30.0f;

	//最大の距離
	m_MaxCameraDistance = m_Camera->GetDistance();

	// カメラの最初の位置
	m_FirstCameraPos = m_Camera->GetPos();

	// プレイヤーリスト取得
	auto Players = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);

	// プレイヤー全体で最小と最大の位置を取得
	for (auto ite : Players)
	{
		std::weak_ptr<CPlayer> Player = std::dynamic_pointer_cast<CPlayer>(ite);
		m_Players.push_back(Player);
	}

	/* CPUタイプのオブジェクトリストを参照 */
	const auto& rListCPU = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::CPU);

	/* CPUを弱参照を保有*/
	for (const auto& rIt : rListCPU)
	{
		std::weak_ptr<CEnemyPlayer> wpCPU = std::dynamic_pointer_cast<CEnemyPlayer>(rIt);
		m_vwpCPUs.push_back(wpCPU);
	}

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
	// 既に削除されてるプレイヤーを除外
	RemoveExpiredPlayers();

	// ギミックを取得
	GetObstacles();

	// 外周を移動するギミックがあるか
	HasPerimeterGimmick();

	if (!m_IsPerimeterGimmickActive)
	{
		// 移動ギミックがあるか判定
		HasMovingGimmick();

		if (!m_IsMovingGimmickActive)
		{
			// カメラの注視点位置設定
			CalculateCenter();
		}
	}
	
	// カメラの変更位置を設定
	m_Camera->SetPosTarget(m_CameraTargetPos);

	// 保存したギミックの情報全部消す
	m_Obstacles.clear();
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

	// 距離設定
	m_Camera->SetDistanceTarget(m_MaxCameraDistance);

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
		auto Player = ite.lock();

		if (InRange(Player.get()->GetTransform().Pos))
		{
			if (Count == 0)
			{
				MinPlayersPos = Player->GetTransform().Pos;
				MaxPlayersPos = Player->GetTransform().Pos;
			}

			// X座標の最小最大
			MaxPlayersPos.x = max(MaxPlayersPos.x, Player->GetTransform().Pos.x);
			MinPlayersPos.x = min(MinPlayersPos.x, Player->GetTransform().Pos.x);

			// Z座標の最小最大
			MaxPlayersPos.z = max(MaxPlayersPos.z, Player->GetTransform().Pos.z);
			MinPlayersPos.z = min(MinPlayersPos.z, Player->GetTransform().Pos.z);
		}
		
		Count++;
	}

	/* CPU全体で最小と最大の位置を取得 */
	for (const auto& rIt : m_vwpCPUs)
	{
		/* CPUが存在すれば */
		if (std::shared_ptr<CEnemyPlayer> spCPU = rIt.lock())
		{
			if (InRange(spCPU.get()->GetTransform().Pos))
			{
				/* X座標の最小最大 */
				MaxPlayersPos.x = max(MaxPlayersPos.x, spCPU->GetTransform().Pos.x);
				MinPlayersPos.x = min(MinPlayersPos.x, spCPU->GetTransform().Pos.x);

				/* Z座標の最小最大 */
				MaxPlayersPos.z = max(MaxPlayersPos.z, spCPU->GetTransform().Pos.z);
				MinPlayersPos.z = min(MinPlayersPos.z, spCPU->GetTransform().Pos.z);
			}
		}
	}

	// ギミックで最小と最大の位置を取得
	for (auto ite : m_Obstacles)
	{
		if (ite->GetObsType() != Obstacle::OBSTACLE_TYPE::STATIONARY)
		{
			continue;
		}

		if (InRange(ite->GetTransform().Pos))
		{
			// X座標の最小最大
			MaxPlayersPos.x = max(MaxPlayersPos.x, ite->GetTransform().Pos.x);
			MinPlayersPos.x = min(MinPlayersPos.x, ite->GetTransform().Pos.x);

			// Z座標の最小最大
			MaxPlayersPos.z = max(MaxPlayersPos.z, ite->GetTransform().Pos.z);
			MinPlayersPos.z = min(MinPlayersPos.z, ite->GetTransform().Pos.z);
		}
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
	auto ListRaw = CObjectManager::RefInstance().RefListRaw(OBJ::TYPE::OBSTACLE);
	auto ListShare = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::OBSTACLE);

	for (auto ite : ListRaw)
	{
		CObstacle* Obstacle = dynamic_cast<CObstacle*>(ite);

		m_Obstacles.push_back(Obstacle);

	}

	for (auto ite : ListShare)
	{
		CObstacle* Obstacle = dynamic_cast<CObstacle*>(ite.get());

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

		// 距離設定
		m_Camera->SetDistanceTarget(m_MaxCameraDistance);
	}
}

//============================================================================
// 外周移動ギミック
//============================================================================
void CCameraController::HasPerimeterGimmick()
{
	// カメラの距離
	constexpr float Distance = 50.0f;

	// 判定を戻す
	m_IsPerimeterGimmickActive = false;

	for (auto ite : m_Obstacles)
	{
		if (ite->GetObsType() == Obstacle::OBSTACLE_TYPE::PERIMETER)
		{
			m_IsPerimeterGimmickActive = true;
		}
	}

	// 外周移動ギミックない
	if (m_IsPerimeterGimmickActive)
	{
		// 最初の位置に戻る
		m_CameraTargetPos = m_FirstCameraPos;

		// 距離設定
		m_Camera->SetDistanceTarget(Distance);
	}
}

//============================================================================
// 既に削除されてるプレイヤーを除外
//============================================================================
void CCameraController::RemoveExpiredPlayers()
{
	// weak_ptrがExpiredかどうか判定
	auto IsExpired =
		[](const std::weak_ptr<CPlayer>& wp)
	{
		return wp.expired();
	};

	// Expiredな要素を後方に移動して整理
	auto newEnd = std::remove_if(m_Players.begin(), m_Players.end(), IsExpired);

	// 条件にあうものを削除
	m_Players.erase(newEnd, m_Players.end());
}

//============================================================================
// 範囲内にいるか
//============================================================================
bool CCameraController::InRange(DirectX::XMFLOAT3 pos)
{
	// 地面を取得
	auto ListShare = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::FIELD);

	CField* Field = nullptr;

	for (auto ite : ListShare)
	{
		Field = dynamic_cast<CField*>(ite.get());
	}
	
	const float fSpanField = 15.0f;	// 地面の大きさ
	const float Range = 0.0f;		// 範囲

	if (pos.x >= Field->GetTransform().Pos.x + fSpanField + Range
		|| pos.x <= Field->GetTransform().Pos.x - fSpanField - Range
		|| pos.z >= Field->GetTransform().Pos.z + fSpanField + Range
		|| pos.z <= Field->GetTransform().Pos.z - fSpanField - Range)
	{
		return false;
	}

	return true;
}
