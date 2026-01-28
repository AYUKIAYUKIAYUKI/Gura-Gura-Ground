//============================================================================
// 
// タル [Barrel.cpp]
// Author : 元地弘汰
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "Barrel.h"
#include "API.gltf.manager.h"
#include "API.object.manager.h"
#include "player.h"
#include "API.collision.h"

// 物理挙動作成のため
#include "API.world.h"
#include "API.ghost.h"

// エフェクト
#include "shadow.h"
#include "route.h"
#include "dust.h"

//オイル生成する
#include "Oil.h"

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
	float g_fAxisY_Despawn = 3.0f;

	// その他設定値
	float ShpireHalf = 3.0f;		//生成球半径
	float SideHeight = 1.0f;		//生成高さ
	int g_OilCreateSpan = 30;


	// 位置表示
	void Print_Pos(const OBJ::Transform& TF)
	{

	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CBarrel::CBarrel(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CObstacle(Type, Layer, Obstacle::OBSTACLE_TYPE::MOVING)
	, m_nTimer(0)
	, m_fHalfSize(0.0f)
{
	// モデルのセット
	SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Barrel"));
}

//============================================================================
// デストラクタ
//============================================================================
CBarrel::~CBarrel()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CBarrel::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// デフォルトのリジットボディを生成
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::CYLINDER, fWidth, fHeight, fDepth));
	m_fHalfSize = fHeight;

	// コライダーをリジットボディにキャスト
	CRigidBody* const pRb = dynamic_cast<CRigidBody*>(GetCollider());
	pRb->SetMass(1000.0f);

	/* ！！！ 影の生成 ！！！ */
	CShadow* pShadow = CObjectManager::CreateRaw<CShadow>(OBJ::TYPE::NONE, OBJ::LAYER::DEFAULT);
	pShadow->SetTrackTarget(shared_from_this());

	/* ！！！ 警告表示の作成 ！！！ */
	CRoute* pRoute = CObjectManager::CreateRaw<CRoute>();
	std::shared_ptr<CObstacle> spObstacle = std::dynamic_pointer_cast<CObstacle>(shared_from_this());
	pRoute->SetTrackTarget(spObstacle);
}

//============================================================================
// 更新処理
//============================================================================
void CBarrel::Update()
{
	// 挙動
	Action();


	// ワールドトランスフォームから位置を取得
	CRigidBody* const pRB = useful::DownCast<CRigidBody>(GetCollider());
	const DirectX::XMFLOAT3& Pos = pRB->GetWorldTransform().Pos;
	if (Pos.y < 3.0f)
	{
		// 自身の死亡フラグを立てる
		SetDeath();
	}
	
	// 障害物クラスの更新
	CObstacle::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CBarrel::Draw()
{
	// 障害物クラスの描画
	CObstacle::Draw();
}

//============================================================================
// 挙動
//============================================================================
void CBarrel::Action()
{
	// 進行方向をコピー
	btVector3 MoveDir = { m_Direction.x, m_Direction.y, m_Direction.z };

	// リジッドボディの取得
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// 現在の加速度をコピー
	const btVector3& rCurrentVel = pRB->GetLinearVelocity();

	// リジッドボディのアクティブ化
	pRB->SetActive();

	// 移動方向：Y軸：現在の重力速度を維持
	MoveDir.setY(rCurrentVel.getY());

	// 新しい加速度をセット
	pRB->SetLinearVelocity(MoveDir);
	
	
	//一定間隔でオイルを生成
	m_nTimer++;
	if (m_nTimer > g_OilCreateSpan)	{
		m_nTimer = 0;
		const float HalfSize = m_fHalfSize;
		XMFLOAT3 Position = GetTransform().Pos;

		CObjectManager::CreateRaw<COil>([HalfSize, Position](COil* p)->bool {
			p->FactoryCollider(1.0f, HalfSize,1.0f);
			const CGhost* const pRB = useful::DownCast<CGhost>(p->GetCollider());
			OBJ::Transform TF = p->GetTransform();
			TF.Pos = { Position.x,6.0f,Position.z };
			p->SetTransform(TF);
			pRB->SetWorldTransform(TF);
			return true;
			});
	}

}

// 進行方向に応じて向きを変更
void CBarrel::SetRotate(OBJ::Transform& rTF, XMFLOAT3 Dir)
{
	// 回転方向を作成
	btQuaternion RotateVec = {};

	/* ちょっとひどいです */
	if (Dir.x > 0.0f)
	{
		// 左右移動の場合、Z方向に回転
		RotateVec.setEulerZYX(0.0f, 0.0f, 3.1415927f * 0.5f);
	}
	else
	{
		// 前後移動の場合、X方向に回転
		RotateVec.setEulerZYX(3.1415927f * 0.5f, 0.0f, 0.0f);
	}

	// 方向を正規化
	RotateVec.normalize();

	// トランスフォームに回転を反映
	rTF.Rot = { RotateVec.getX(), RotateVec.getY(), RotateVec.getZ(), RotateVec.getW() };
}