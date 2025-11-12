//============================================================================
// 
// ギミックマネージャー [gimmick.manager.cpp]
// Author : 後藤優輝
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "gimmick.manager.h"
#include "IronBall.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CGimmickManager::CGimmickManager():
	m_SpawnTime(0),
	m_WarningActive(false)
{}

//============================================================================
// デストラクタ
//============================================================================
CGimmickManager::~CGimmickManager()
{}

//============================================================================
// 初期化処理
//============================================================================
bool CGimmickManager::Initialize()
{
	// ギミック出現時間
	constexpr int SPAWN_TIME = 300;

	// ギミック発生条件の設定
	SpawnCondition SCondition;

	// 条件
	SCondition.trigger = "time";

	// 生成する経過時間
	SCondition.triggerTime = SPAWN_TIME;
	
	// 生成判定
	SCondition.IsSpawned = false;

	// ギミック削除条件の設定
	DeleteCondition DCondition;
	
	// 条件
	DCondition.trigger = "time";

	// 削除する経過時間
	DCondition.triggerTime = SPAWN_TIME;

	// 削除判定
	DCondition.IsDelete = false;

	//ギミック関する情報を設定
	m_GimmickEntry.DeleteCondition = DCondition;	// 削除条件を設定
	m_GimmickEntry.SpawnConditions = SCondition;	// 生成条件を設定
	m_GimmickEntry.Gimmick = nullptr;				// 生成しない

	return true;
}

//============================================================================
// 終了処理
//============================================================================
void CGimmickManager::Finalize()
{

}

//============================================================================
// 更新処理
//============================================================================
void CGimmickManager::Update()
{
	m_WarningActive = false;
	m_SpawnTime++;

	// ギミックを出現させるか判定
	CanSpawnGimmick();

	// ギミックを消す
	GimmickDelete();
}

//============================================================================
// 生成処理
//============================================================================
void  CGimmickManager::Create()
{
	// 鉄球の生成
	auto Gimmick = CObject::Create<CIronBall>(OBJ::TYPE::NONE, OBJ::LAYER::DEFAULT, CIronBall::s_fpDefaultFactory);
	m_GimmickEntry.Gimmick = Gimmick;
	m_GimmickEntry.SpawnConditions.IsSpawned = true;

}

//============================================================================
// ギミックを出現させるか判定
//============================================================================
void CGimmickManager::CanSpawnGimmick()
{
	// ギミック出現の〇秒前に警告を出す時間
	constexpr int WARNING_FRAME = 120;

	//for (auto it = m_SpawnConditions.begin(); it != m_SpawnConditions.end();)
	//{
		if (CanSpawnCondition(m_GimmickEntry.SpawnConditions))
		{//条件を満たしてるとき

			// 生成
			Create();

			//it = m_SpawnConditions.erase(it);
		}
		else
		{
			//出現時間
			int TriggerTime = m_GimmickEntry.SpawnConditions.triggerTime;

			if (!m_WarningActive								 // 警告中ではない
				&& m_SpawnTime >= (TriggerTime - WARNING_FRAME)) // かつ出現時間の2秒前
			{
				m_WarningActive = true;
			}

			//it++;
		}
	//}
}

//============================================================================
// 出現条件を満たしてるか
//============================================================================
bool CGimmickManager::CanSpawnCondition(const SpawnCondition& Condition)
{
	if (Condition.trigger == "time")
	{//時間経過が条件のとき

		if (m_SpawnTime >= Condition.triggerTime
			&& !Condition.IsSpawned)
		{
			return true;
		}
	}

	return false;
}

//============================================================================
// ギミックを消す
//============================================================================
void CGimmickManager::GimmickDelete()
{
	if (m_GimmickEntry.DeleteCondition.trigger == "time")
	{//時間経過が条件のとき

		if (m_GimmickEntry.DeleteCondition.IsDelete)
		{
			return;
		}

		// 生成時間
		int SpawnTriggerTime = m_GimmickEntry.SpawnConditions.triggerTime;

		// 消去時間
		int DeleteTriggerTime = m_GimmickEntry.DeleteCondition.triggerTime;

		if (m_SpawnTime >= SpawnTriggerTime + DeleteTriggerTime)
		{//消去するまでの時間が経過した

			m_GimmickEntry.Gimmick->SetDeath();
			m_GimmickEntry.DeleteCondition.IsDelete = true;
			m_GimmickEntry.Gimmick = nullptr;
		}
	}

}