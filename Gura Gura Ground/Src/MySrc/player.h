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

//****************************************************
// 前方宣言
//****************************************************
class CField;
class FallTetra_Behavior;

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
	static constexpr size_t MAX_PLAYER_COUNT = 4;

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

	static std::vector<float> s_vSurvivalTimes; //生存時間 

	//ぺちゃんこ状態の管理
	std::shared_ptr<FallTetra_Behavior> GetFallTetraBehavior() { return m_pFallTetraBehavior; }
	//外部からぺちゃんこ有効化するための関数
	void EnableFallTetraBehavior() {
		if (m_pFallTetraBehavior != nullptr)m_pFallTetraBehavior.reset();
		m_pFallTetraBehavior = std::make_shared<FallTetra_Behavior>();
	}

	//プレイヤーごとの生存時間取得
	static float GetSurvivalTimeForIndex(size_t idx) {if (idx < s_vSurvivalTimes.size()) return s_vSurvivalTimes[idx];return 0.0f;}
	//生存時間削除
	static void ClearAllSurvivalTimes() { std::fill(s_vSurvivalTimes.begin(), s_vSurvivalTimes.end(), 0.0f); }

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

	std::shared_ptr<FallTetra_Behavior> m_pFallTetraBehavior;
protected:
	bool m_bIsDead; //死亡したかどうか
};

//仮でドッスン関連の挙動実装するクラス追加 ←結局本実装になるやつ
class FallTetra_Behavior
{
public:
	FallTetra_Behavior() :m_Timer(180), m_DecayValue(0.3f){}
	float GetDecayValue() { return m_DecayValue; }
	void SetDecayValue(float v) { m_DecayValue = v; }
	bool GetTimer() {
		--m_Timer;
		return m_Timer > 0;
	}
	void TimerReset() { m_Timer = 180; }
private:
	float m_DecayValue;
	int m_Timer;
};