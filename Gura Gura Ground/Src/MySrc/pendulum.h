
//============================================================================
// 
// 振り子 [pendulum.h]
// Author : 大竹熙
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "obstacle.h"
#include "player.h"
#include "API.rigidbody.h"
#include <unordered_set>

//****************************************************
// バークラスの定義
//****************************************************
class CPendulum : public CObstacle
{
public:

	//****************************************************
	// special function
	//****************************************************
	CPendulum(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CPendulum() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// コライダーのファクトリ
	void FactoryCollider(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f) override;

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// パラメータの編集
	void EditParam() override { int i = 0; }

	// 進行方向の設定
	inline const DirectX::XMFLOAT3& GetDirection() const { return m_Direction; }
	inline       void               SetDirection(const DirectX::XMFLOAT3& Direction) { m_Direction = Direction; }

private:

	//****************************************************
	// function
	//****************************************************
	void Appear(); // 出現
	void Action(); // 挙動
	void Loop();   // 戻る
	void CheckHitPlayer();
	void CreateHingeConstraint(CRigidBody* rb, float radius);

	//****************************************************
	// data
	//****************************************************
	DirectX::XMFLOAT3 m_Direction;   // 進行方向
	float m_Time;                    // 経過時間
	float m_Phase = 0.0f;            // 揺れの位相
	int m_HitCooldown = 0;           // ヒットクールタイム
	DirectX::XMFLOAT3 m_prevPos = { 0.0f, 0.0f, 0.0f }; // 前フレーム位置
	DirectX::XMFLOAT3 m_OriginPos = { 0.0f, 0.0f, 0.0f }; // 振り子座標の原点
	bool m_hasPrevPos = false;                          // 初回記録済み
	bool m_CollisionDisabled;
	CRigidBody* m_pRB = nullptr;
	std::unordered_set<CPlayer*> m_HitPlayers;

};