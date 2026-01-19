//============================================================================
// 
// プレイヤー、ヘッダーファイル [player.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.physics.model.h"
#include "debuff_behavior.h"

//****************************************************
// 前方宣言
//****************************************************
class CField;
class Debuff_Behavior;


//****************************************************
// プレイヤークラスの定義
//****************************************************
class CPlayer : public CPhysicsModel
{
	//****************************************************
	// 静的定数
	//****************************************************

	// 塵生成の最大ステップ数
	static constexpr int DUST_STEP_COUNT_MAX = 15;

public:

	//****************************************************
	// 前方宣言
	//****************************************************
	struct StateMachine;

	//****************************************************
	// special function
	//****************************************************
	CPlayer(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CPlayer() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// コライダーのファクトリ
	void FactoryCollider(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f) override;

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// 衝撃波の作成
	void CreateShockWave(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3A& Size, int nDuration);

	// プレイヤーのインデックス操作用
	unsigned char GetIdxPlayer() const;
	void          SetIdxPlayer(unsigned char wIdx);

	// 操作不能期間の操作用
	int  GetLostControlDuration() const;
	void SetLostControlDuration(int nTime);

	// 塵の進行更新
	void UpdateDustStep(const DirectX::XMFLOAT3& Direction);

	//ぺちゃんこ状態の管理
	std::shared_ptr<Debuff_Behavior> GetFallTetraBehavior() { return m_pDebuffBehavior; }
	//外部からデバフ有効化するための関数
	void EnableStamp() {
		if (DB_UseCheck())return;
		m_pDebuffBehavior = std::make_shared<Stamp_DB>();
	}
	void EnableBird() {
		if (DB_UseCheck())return;
		m_pDebuffBehavior = std::make_shared<Bird_DB>();
	}
	void EnableOil() {
		if (DB_UseCheck())return;
		m_pDebuffBehavior = std::make_shared<Oil_DB>();
	}
private:

	//****************************************************
	// function
	//****************************************************
	void CheckDeath(); // 死亡チェック

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<StateMachine> m_upStateMachine;       // 状態管理
	std::weak_ptr<CField>         m_wpField;              // 地面
	unsigned char                 m_wIdxPlayer;           // プレイヤーのインデックス
	int                           m_nLostControlDuration; // 操作不能期間
	int                           m_nStepCounter;         // 進行カウンター

	std::shared_ptr<Debuff_Behavior> m_pDebuffBehavior;

	//既にデバフが有効なら時間だけ戻すよ
	inline bool DB_UseCheck()	{
		if (m_pDebuffBehavior != nullptr) {
			m_pDebuffBehavior.reset();
			return true;
		}
		return false;
	}
};

