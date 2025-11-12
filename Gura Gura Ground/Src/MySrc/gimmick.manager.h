//============================================================================
// 
// ギミックマネージャー、ヘッダーファイル [gimmick.manager.h]
// Author : 後藤優輝
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.singleton.h"
#include "API.object.h"

//****************************************************
// 前方宣言
//****************************************************
class CIronBall;

//****************************************************
// ギミックマネージャークラスの定義
//****************************************************
class CGimmickManager final : public CSingleton<CGimmickManager>
{

	//****************************************************
	// 出現条件を表す構造体
	//****************************************************
	struct SpawnCondition
	{
		int triggerTime;				 // 経過時間で出現
		std::string trigger;			 // 条件
		std::function<bool()> condition; // 出現条件
		bool IsSpawned;					 // 生成されたか
		//DirectX::XMFLOAT3 spawnPosition; // 出現位置
		//std::string objectType;			 // 何を出すか
	};

	//****************************************************
	// 消去条件を表す構造体
	//****************************************************
	struct DeleteCondition
	{
		int triggerTime;               // 経過時間で消去
		std::string trigger;			 // 条件
		std::function<bool()> condition; // 消去条件
		bool IsDelete;					 // 消去されたか
	};

	//****************************************************
	// ギミックの管理をする構造体
	//****************************************************
	struct GimmickEntry
	{
		CIronBall* Gimmick;					// ギミックのポインタ
		SpawnCondition SpawnConditions;		// 出現条件
		DeleteCondition DeleteCondition;	// 消去条件
	};

	//****************************************************
	// フレンド宣言
	//****************************************************
	friend struct std::default_delete<CGimmickManager>;
	friend CGimmickManager& CSingleton<CGimmickManager>::RefInstance();

public:

	//****************************************************
	// function
	//****************************************************

	// 更新処理
	void Update();
	
	// 警告中かの判定を取得
	bool GetWarningActive()const { return m_WarningActive; }
private:

	//****************************************************
	// special function
	//****************************************************
	CGimmickManager();           // デフォルトコンストラクタ
	~CGimmickManager() override; // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// 初期化処理
	bool Initialize();

	// 終了処理
	void Finalize();

	// 生成
	void Create();

	// ギミックを出現させるか判定
	void CanSpawnGimmick();

	// 出現条件を満たしてるか
	bool CanSpawnCondition(const SpawnCondition& Condition);

	// ギミックを消す
	void GimmickDelete();

	//****************************************************
	// data
	//****************************************************
	std::list<CObject*> m_GimmickList;				// ギミックを格納
	std::vector<SpawnCondition> m_SpawnConditions;	// 出現条件を格納
	GimmickEntry m_GimmickEntry;					// ギミックの管理
	int m_SpawnTime;								// 時間経過カウント
	bool m_WarningActive;							// 警告表示中か
};