//============================================================================
// 
// 衝撃波 [shockwave.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "shockwave.h"

// 当たり判定用
#include "API.collision.h"
#include "API.object.manager.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CShockWave::CShockWave(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CPhysicsObject(Type, Layer)
	, m_wpIgnore()
	, m_nDuration(0)
{}

//============================================================================
// デストラクタ
//============================================================================
CShockWave::~CShockWave()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CShockWave::FactoryCollider(Collision::SHAPETYPE Type, float fWidth, float fHeight, float fDepth)
{
	// ゴーストの作成
	SetCollider(CGhost::CreateGhost(GetTransform(), Type, fWidth, fHeight, fDepth));
}

//============================================================================
// 更新処理
//============================================================================
void CShockWave::Update()
{
	--m_nDuration;

	// 期間が無くなれば死亡フラグを立てる
	if (m_nDuration < 0)
	{
		SetDeath();
	}

	// 押し出し処理
	Push();

	// WVP行列の更新
	CPhysicsObject::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CShockWave::Draw()
{
	// モデルの描画
	CPhysicsObject::Draw();
}

//============================================================================
// 押し出し処理
//============================================================================
void CShockWave::Push()
{
	// 衝撃波のコライダーをゴーストにキャスト
	CGhost* pShockwaveGhost = useful::DownCast<CGhost>(GetCollider());

	// 無視対象のリジッドボディ用シェアポインタ
	CRigidBody* pIgnore = nullptr;

	// 無視対象が物理オブジェクトにキャスト出来たら
	if (std::shared_ptr<CPhysicsObject> spPhysicsObject = std::dynamic_pointer_cast<CPhysicsObject>(m_wpIgnore.lock()))
	{
		// コライダーをリジッドボディにキャストしておく
		pIgnore = dynamic_cast<CRigidBody*>(spPhysicsObject->GetCollider());
	}

	// コールバックの定義
	const auto& fpCallBack = [&pShockwaveGhost, &pIgnore](const std::shared_ptr<CObject>& spObject) -> bool
		{
			// 物理オブジェクトにキャスト出来たら
			if (std::shared_ptr<CPhysicsObject> spPhysicsObject = std::dynamic_pointer_cast<CPhysicsObject>(spObject))
			{
				// コライダーをリジッドボディにキャスト出来たら
				if (CRigidBody* pRigidBodyGet = dynamic_cast<CRigidBody*>(spPhysicsObject->GetCollider()))
				{
					// 無視対象のリジッドボディならスキップ
					if (pIgnore == pRigidBodyGet)
					{
						return false;
					}

					// リジッドボディを持っていたら
					Collision::BumperPush(pShockwaveGhost, pRigidBodyGet, 3.0f);
				}
			}

			return true;
		};

	// シェアポインタのオブジェクトを走査
	CObjectManager::RefInstance().ForEachShare(fpCallBack);
}