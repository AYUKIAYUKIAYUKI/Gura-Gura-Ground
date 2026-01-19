//============================================================================
// 
// オイル [Oil.cpp]
// Author : 元地弘汰
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "Oil.h"
#include "API.object.manager.h"
#include "player.h"
#include "API.collision.h"

// 物理挙動作成のため
#include "API.world.h"
#include "API.ghost.h"

// エフェクト
#include "dust.h"

//****************************************************
// usingディレクティブ
//****************************************************
using namespace DirectX;
using namespace useful;

//****************************************************
// 無名名前空間の定義
//****************************************************
namespace
{
	// フィールドサイズ
	float g_fFieldSpan = 15.0f;
	float g_fFieldHalf = g_fFieldSpan * 0.5f;
	
	// その他設定値
	float CylinderHeight = 0.5f;	//生成球半径
	float Cylinderhalf = 0.1f;		//生成高さ

	int g_nDeathTimer = 300;

	// 位置表示
	void Print_Pos(const OBJ::Transform& TF)
	{

	}
	
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
COil::COil(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CObstacle(Type, Layer, Obstacle::OBSTACLE_TYPE::MOVING)
	, m_nTimer(0)
{}

//============================================================================
// デストラクタ
//============================================================================
COil::~COil()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void COil::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// デフォルトのゴーストを生成
	SetCollider(CGhost::CreateGhost(GetTransform(), Collision::SHAPETYPE::CYLINDER, fHeight, CylinderHeight, fHeight));
		
	// コライダーをゴーストにキャスト
	CGhost* const pGs = dynamic_cast<CGhost*>(GetCollider());

}

//============================================================================
// 更新処理
//============================================================================
void COil::Update()
{
	// 挙動
	Action();

	//プレイヤーとの接触
	ToPlayer();

	// 物理オブジェクト用の更新：WVP行列用定数バッファの更新
	CPhysicsObject::Update();
}

//============================================================================
// 描画処理
//============================================================================
void COil::Draw()
{
	// 物理オブジェクト用の描画：モデルの描画
	CPhysicsObject::Draw();
}

//============================================================================
// 挙動
//============================================================================
void COil::Action()
{
	m_nTimer++;
	if (m_nTimer > g_nDeathTimer)SetDeath();
}

//============================================================================
// プレイヤーとの接触
//============================================================================
void COil::ToPlayer()
{
	CGhost* const pGs = dynamic_cast<CGhost*>(GetCollider());
	// オブジェクトのリストを取得
	const auto& rPlayerList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);

	for (const auto& e : rPlayerList)
	{
		std::shared_ptr<CPlayer> pPlayerObj = std::dynamic_pointer_cast<CPlayer>(e);

		// プレイヤー型にキャスト可能なら
		if (pPlayerObj)
		{
			CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(pPlayerObj->GetCollider());
			// 衝突判定
			
			Collision::MyContactCallbackGhostAndRigidBody CallBack(pGs, pRigidBody);
			CWorld::RefInstance().RefDynamicsWorldConst()->contactPairTest(pGs->GetGhost(), pRigidBody->GetRigidBody(), CallBack);

			// 衝突が確認出来たら
			if (CallBack.m_bHit)
			{
				pPlayerObj->EnableFallTetraBehavior();
			}
		}
	}
}