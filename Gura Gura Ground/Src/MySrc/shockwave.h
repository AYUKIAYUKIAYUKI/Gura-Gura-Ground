//============================================================================
// 
// 衝撃波、ヘッダーファイル [shockwave.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.collider.h"
#include "API.physics.object.h"

//****************************************************
// 前方宣言
//****************************************************
class CPlayer;

//****************************************************
// 衝撃波クラスの定義
//****************************************************
class CShockWave : public CPhysicsObject
{
public:

	//****************************************************
	// special function
	//****************************************************
	CShockWave(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CShockWave() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// コライダーのファクトリ
	void FactoryCollider(Collision::SHAPETYPE Type, float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// 期間の設定
	inline void SetDuration(int nDuration) { m_nDuration = nDuration; }

	// プレイヤーの設定
	inline void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }

private:

	//****************************************************
	// data
	//****************************************************
	CPlayer* m_pPlayer;   // プレイヤーの
	int      m_nDuration; // 期間
};