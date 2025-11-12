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
	SCondition.trigger = "time";			// ギミック発生条件
	SCondition.triggerTime = SPAWN_TIME;	// 生成する経過時間

	// ギミック削除条件の設定
	DeleteCondition DCondition;
	DCondition.trigger = "time";			// ギミック削除条件
	DCondition.triggerTime = SPAWN_TIME;	// 削除する経過時間

	//ギミック関する情報を設定
	GimmickEntry GimmickEntry;
	GimmickEntry.DeleteCondition = DCondition;	// 削除条件を設定
	GimmickEntry.SpawnConditions = SCondition;	// 生成条件を設定
	GimmickEntry.Gimmick = nullptr;				// 生成しない
	GimmickEntry.IsSpawned = false;				// 生成されていない
	
	//格納
	m_GimmickEntryList.push_back(GimmickEntry);

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
	// 警告表示してない状態に変更
	m_WarningActive = false;

	m_SpawnTime++;

	// ギミックを出現させる
	GimmickSpawn();

	// ギミックを消す
	GimmickDelete();
}

//============================================================================
// 生成処理
//============================================================================
void  CGimmickManager::Create(GimmickEntry& GimmickEntry)
{
	// 鉄球の生成
	auto Gimmick = CObject::Create<CIronBall>(OBJ::TYPE::NONE, OBJ::LAYER::DEFAULT, CIronBall::s_fpDefaultFactory);
	GimmickEntry.Gimmick = Gimmick;	// 生成したインスタンスのアドレスを代入
	GimmickEntry.IsSpawned = true;	// 生成されている状態に変更
}

//============================================================================
// ギミックを出現させる
//============================================================================
void CGimmickManager::GimmickSpawn()
{
	// ギミック出現の〇秒前に警告を出す時間
	constexpr int WARNING_FRAME = 120;

	for (auto& it : m_GimmickEntryList)
	{
		if (CanSpawnCondition(it.SpawnConditions)	// 出現条件を満たしてる
			&&!it.IsSpawned)						// 生成されてない
		{

			// 生成
			Create(it);
		}
		else
		{
			//出現時間
			int TriggerTime = it.SpawnConditions.triggerTime;

			if (!m_WarningActive								 // 警告中ではない
				&& m_SpawnTime >= (TriggerTime - WARNING_FRAME)) // かつ出現時間の2秒前
			{
				//警告表示中に変更
				m_WarningActive = true;
			}
		}
	}
}

//============================================================================
// 出現条件を満たしてるか
//============================================================================
bool CGimmickManager::CanSpawnCondition(SpawnCondition& SCondition)
{
	SpawnCondition Condition = SCondition;

	if (Condition.trigger == "time")
	{//時間経過が条件のとき

		if (m_SpawnTime >= Condition.triggerTime)
		{// 出現時間経過

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
	for (auto it = m_GimmickEntryList.begin(); it != m_GimmickEntryList.end();)
	{
		if (!it->IsSpawned)
		{//生成してない
			it++;
			continue;
		}

		if (it->DeleteCondition.trigger == "time")
		{//時間経過が条件のとき

			// 生成時間
			int SpawnTriggerTime = it->SpawnConditions.triggerTime;

			// 消去時間
			int DeleteTriggerTime = it->DeleteCondition.triggerTime;

			if (m_SpawnTime >= SpawnTriggerTime + DeleteTriggerTime)
			{//消去するまでの時間が経過した

				//ギミックを削除
				it->Gimmick->SetDeath();
				it->Gimmick = nullptr;

				it = m_GimmickEntryList.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}