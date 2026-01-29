////============================================================================
//// 
//// ウソのプレイヤー、ヘッダーファイル [player.h]
//// Author : 福田歩希
//// 
////============================================================================
//
//#pragma once
//
////****************************************************
//// インクルードファイル
////****************************************************
//#include "API.physics.model.h"
//
////****************************************************
//// ウソのプレイヤークラスの定義
////****************************************************
//class CPlayerFake : public CPhysicsModel
//{
//public:
//
//	//****************************************************
//	// 前方宣言
//	//****************************************************
//	struct StateMachine;
//
//	//****************************************************
//	// special function
//	//****************************************************
//	CPlayerFake(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
//	~CPlayerFake() override;                       // デストラクタ
//
//	//****************************************************
//	// function
//	//****************************************************
//
//	// コライダーのファクトリ
//	void FactoryCollider(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f) override;
//
//	// 更新処理
//	void Update() override;
//
//	// 描画処理
//	void Draw() override;
//
//	// 衝撃波の作成
//	void CreateShockWave(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3A& Size, int nDuration);
//
//	// プレイヤーのインデックス操作用
//	unsigned char GetIdxPlayer() const;
//	void          SetIdxPlayer(unsigned char wIdx);
//
//	// 塵の進行更新
//	void UpdateDustStep(const DirectX::XMFLOAT3& Direction);
//
//private:
//
//	//****************************************************
//	// data
//	//****************************************************
//	std::unique_ptr<StateMachine> m_upStateMachine; // 状態管理
//	unsigned char                 m_wIdxPlayer;     // プレイヤーのインデックス
//	int                           m_nStepCounter;   // 停止期間
//};