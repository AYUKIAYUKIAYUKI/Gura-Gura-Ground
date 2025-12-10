//============================================================================
// 
// ボム衝撃波 [bombshockwave.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "bombshockwave.h"

// コライダーの作成用
#include "API.ghost.h"

// 衝突判定用
#include "API.collision.h"
#include "player.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CBombShockWave::CBombShockWave(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CPhysicsObject(Type, Layer)
	, m_nDuration(0)
{}

//============================================================================
// デストラクタ
//============================================================================
CBombShockWave::~CBombShockWave()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CBombShockWave::FactoryCollider(Collision::SHAPETYPE Type, float fWidth, float fHeight, float fDepth)
{
	// ゴーストの作成
	SetCollider(CGhost::CreateGhost(GetTransform(), Type, fWidth, fHeight, fDepth));
}

//============================================================================
// 更新処理
//============================================================================
void CBombShockWave::Update()
{
	// ひどい
	/*----------------------------*/

	CGhost* pGhost = useful::DownCast<CGhost>(GetCollider());

	// オブジェクトのリストを取得
	const auto& rObjList = CObjectManager::RefInstance().RefObjList(OBJ::TYPE::PLAYER);

	for (const auto& rIt : rObjList)
	{
		CPlayer* pPlayer = dynamic_cast<CPlayer*>(rIt);

		// プレイヤーにキャスト可能
		if (pPlayer)
		{
			CRigidBody* pRigidBody1 = dynamic_cast<CRigidBody*>(pPlayer->GetCollider());

			// リジッドボディにキャスト可能なら
			if (pRigidBody1)
			{
				Collision::BumperPush(pGhost, pRigidBody1, 1.0f);
			}
		}
	}

	--m_nDuration;

	// 期間が無くなれば死亡フラグを立てる
	if (m_nDuration < 0)
	{
		SetDeath();
	}

	// WVP行列の更新
	CPhysicsObject::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CBombShockWave::Draw()
{
	// モデルの描画
	CPhysicsObject::Draw();
}