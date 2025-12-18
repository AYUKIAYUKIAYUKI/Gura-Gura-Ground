
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// コリジョン、ヘッダーファイル [collision.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.object.manager.h"
#include "API.world.h"
#include "API.physics.object.h"
#include "API.collider.h"
#include "API.rigidbody.h"
#include "API.ghost.h"

#pragma warning(push)
#pragma warning(disable: 4100)

//****************************************************
// 名前空間Collsion：衝突判定にまつわる定義を配置します
//****************************************************
namespace Collision
{
	//****************************************************
	// 衝突判定のコールバック構造体の定義：リジッドボディ対リジッドボディ
	//****************************************************
	struct MyContactCallbackRigidBodyAndRigidBody : public btCollisionWorld::ContactResultCallback
	{
		// デフォルトコンストラクタ
		MyContactCallbackRigidBodyAndRigidBody(CRigidBody* pRigidBody0, CRigidBody* pRigidBody1)
			: m_pRigidBody0(pRigidBody0)
			, m_pRigidBody1(pRigidBody1)
			, m_bHit(false)
		{}

		// デストラクタ
		~MyContactCallbackRigidBodyAndRigidBody() override = default;

		// 衝突時の処理
		virtual btScalar addSingleResult(btManifoldPoint& cp,
			const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
			const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override
		{
			// 衝突判定のみ保持
			m_bHit = true;

			return 0;
		}

		//****************************************************
		// data
		//****************************************************
		CRigidBody* m_pRigidBody0;
		CRigidBody* m_pRigidBody1;
		bool        m_bHit;
	};

	//****************************************************
	// 衝突判定のコールバック構造体の定義：ゴースト対リジッドボディ
	//****************************************************
	struct MyContactCallbackGhostAndRigidBody : public btCollisionWorld::ContactResultCallback
	{
		// デフォルトコンストラクタ
		MyContactCallbackGhostAndRigidBody(CGhost* pGhost, CRigidBody* pRigidBody)
			: m_pGhost(pGhost)
			, m_pRigidBody(pRigidBody)
			, m_bHit(false)
		{}

		// デストラクタ
		~MyContactCallbackGhostAndRigidBody() override = default;

		// 衝突時の処理
		virtual btScalar addSingleResult(btManifoldPoint& cp,
			const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
			const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override
		{
			// 衝突判定のみ保持
			m_bHit = true;

			return 0;
		}

		//****************************************************
		// data
		//****************************************************
		CGhost*     m_pGhost;
		CRigidBody* m_pRigidBody;
		bool        m_bHit;
	};

	// リジッドボディとリジッドボディの衝突判定 - 生ポインタのオブジェクトリストを走査
	static CRigidBody* CheckHitToRigidBodyRaw(CRigidBody* pRigidBody0)
	{
		// オブジェクトのリストを取得
		const std::array<std::list<CObject*>, static_cast<unsigned char>(OBJ::TYPE::MAX)>& rListRaw = CObjectManager::RefInstance().RefListRaw();

		// 全てのリストを走査
		for (const std::list<CObject*>& rTypeList : rListRaw)
		{
			for (const auto& rIt : rTypeList)
			{
				// 物理オブジェクトにキャスト可能で
				if (CPhysicsObject* pPhysicsObject = dynamic_cast<CPhysicsObject*>(rIt))
				{
					// そのコライダーをリジッドボディにキャスト可能なら
					if (CRigidBody* pRigidBody1 = dynamic_cast<CRigidBody*>(pPhysicsObject->GetCollider()))
					{
						// この時、同じリジッドボディ同士の衝突判定は行わない
						if (pRigidBody0 == pRigidBody1)
						{
							continue;
						}

						// 衝突判定を行い
						Collision::MyContactCallbackRigidBodyAndRigidBody CallBack(pRigidBody0, pRigidBody1);
						CWorld::RefInstance().RefDynamicsWorldConst()->contactPairTest(pRigidBody0->GetRigidBody(), pRigidBody1->GetRigidBody(), CallBack);

						// 1つでも衝突が確認出来た段階でそのリジッドボディを返す
						if (CallBack.m_bHit)
						{
							return pRigidBody1;
						}
					}
				}
			}
		}

		return nullptr;
	}

	// リジッドボディとリジッドボディの衝突判定 - シェアポインタのオブジェクトリストを走査
	static CRigidBody* CheckHitToRigidBodyShare(CRigidBody* pRigidBody0)
	{
		// 共有ポインタのオブジェクトのリストを取得
		const std::array<std::list<std::shared_ptr<CObject>>, static_cast<unsigned char>(OBJ::TYPE::MAX)>& rListShare = CObjectManager::RefInstance().RefListShare();

		// 全てのリストを走査
		for (const std::list<std::shared_ptr<CObject>>& rTypeList : rListShare)
		{
			for (const std::shared_ptr<CObject>& rIt : rTypeList)
			{
				// 物理オブジェクトにキャスト可能で
				if (std::shared_ptr<CPhysicsObject> spPhysicsObject = std::dynamic_pointer_cast<CPhysicsObject>(rIt))
				{
					// そのコライダーをリジッドボディにキャスト可能なら
					if (CRigidBody* pRigidBody1 = dynamic_cast<CRigidBody*>(spPhysicsObject->GetCollider()))
					{
						// この時、同じリジッドボディ同士の衝突判定は行わない
						if (pRigidBody0 == pRigidBody1)
						{
							continue;
						}

						// 衝突判定を行い
						Collision::MyContactCallbackRigidBodyAndRigidBody CallBack(pRigidBody0, pRigidBody1);
						CWorld::RefInstance().RefDynamicsWorldConst()->contactPairTest(pRigidBody0->GetRigidBody(), pRigidBody1->GetRigidBody(), CallBack);

						// 1つでも衝突が確認出来た段階でそのリジッドボディを返す
						if (CallBack.m_bHit)
						{
							return pRigidBody1;
						}
					}
				}
			}
		}

		return nullptr;
	}

	// ゴーストとリジッドボディの衝突判定 - 生ポインタのオブジェクトリストを走査
	static CRigidBody* CheckHitToRigidBodyRaw(CGhost* pGhost)
	{
		// オブジェクトのリストを取得
		const std::array<std::list<CObject*>, static_cast<unsigned char>(OBJ::TYPE::MAX)>& rListRaw = CObjectManager::RefInstance().RefListRaw();

		// 全てのリストを走査
		for (const std::list<CObject*>& rTypeList : rListRaw)
		{
			for (const auto& rIt : rTypeList)
			{
				// 物理オブジェクトにキャスト可能で
				if (CPhysicsObject* pPhysicsObject = dynamic_cast<CPhysicsObject*>(rIt))
				{
					// そのコライダーをリジッドボディにキャスト可能なら
					if (CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(pPhysicsObject->GetCollider()))
					{
						// 衝突判定を行い
						Collision::MyContactCallbackGhostAndRigidBody CallBack(pGhost, pRigidBody);
						CWorld::RefInstance().RefDynamicsWorldConst()->contactPairTest(pGhost->GetGhost(), pRigidBody->GetRigidBody(), CallBack);

						// 1つでも衝突が確認出来た段階でそのリジッドボディを返す
						if (CallBack.m_bHit)
						{
							return pRigidBody;
						}
					}
				}
			}
		}

		return nullptr;
	}

	// ゴーストとリジッドボディの衝突判定 - シェアポインタのオブジェクトリストを走査
	static CRigidBody* CheckHitToRigidBodyShare(CGhost* pGhost)
	{
		// 共有ポインタのオブジェクトのリストを取得
		const std::array<std::list<std::shared_ptr<CObject>>, static_cast<unsigned char>(OBJ::TYPE::MAX)>& rListShare = CObjectManager::RefInstance().RefListShare();

		// 全てのリストを走査
		for (const std::list<std::shared_ptr<CObject>>& rTypeList : rListShare)
		{
			for (const std::shared_ptr<CObject>& rIt : rTypeList)
			{
				// 物理オブジェクトにキャスト可能で
				if (std::shared_ptr<CPhysicsObject> spPhysicsObject = std::dynamic_pointer_cast<CPhysicsObject>(rIt))
				{
					// そのコライダーをリジッドボディにキャスト可能なら
					if (CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(spPhysicsObject->GetCollider()))
					{
						// 衝突判定を行い
						Collision::MyContactCallbackGhostAndRigidBody CallBack(pGhost, pRigidBody);
						CWorld::RefInstance().RefDynamicsWorldConst()->contactPairTest(pGhost->GetGhost(), pRigidBody->GetRigidBody(), CallBack);

						// 1つでも衝突が確認出来た段階でそのリジッドボディを返す
						if (CallBack.m_bHit)
						{
							return pRigidBody;
						}
					}
				}
			}
		}

		return nullptr;
	}

	// あ
	static CRigidBody* A(CGhost* pGhost, CRigidBody* pNone)
	{
		// オブジェクトのリストを取得
		const auto& rObjList = CObjectManager::RefInstance().RefListRaw();

		for (const auto& rTypeList : rObjList)
		{
			for (const auto& rIt : rTypeList)
			{
				CPhysicsObject* pPhysicsObject = dynamic_cast<CPhysicsObject*>(rIt);

				// 物理オブジェクトにキャスト可能
				if (pPhysicsObject)
				{
					CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(pPhysicsObject->GetCollider());

					if (pRigidBody == pNone)
					{
						continue;
					}

					// リジッドボディにキャスト可能なら
					if (pRigidBody)
					{
						// 衝突判定
						Collision::MyContactCallbackGhostAndRigidBody CallBack(pGhost, pRigidBody);
						CWorld::RefInstance().RefDynamicsWorldConst()->contactPairTest(pGhost->GetGhost(), pRigidBody->GetRigidBody(), CallBack);

						// 衝突が確認出来たら
						if (CallBack.m_bHit)
						{
							return pRigidBody;
						}
					}
				}
			}
		}

		return nullptr;
	}

	// バンパー反応
	static void BumperPush(CGhost*& rpGhost, CRigidBody*& rpRigidBody, float fPower)
	{
		// 衝突判定
		Collision::MyContactCallbackGhostAndRigidBody CallBack(rpGhost, rpRigidBody);
		CWorld::RefInstance().RefDynamicsWorldConst()->contactPairTest(rpGhost->GetGhost(), rpRigidBody->GetRigidBody(), CallBack);

		// 衝突が確認出来たら
		if (CallBack.m_bHit)
		{
			// お互いの位置を参照
			const DirectX::XMFLOAT3& rGhostPosition     = rpGhost->GetWorldTransform().Pos;
			const DirectX::XMFLOAT3& rRigidBodyPosition = rpRigidBody->GetWorldTransform().Pos;

			// ベースから対象への距離ベクトルを作成
			const btVector3 Direction =
			{
				rRigidBodyPosition.x - rGhostPosition.x,
				//rRigidBodyPosition.y - rGhostPosition.y,
				1.0f,
				rRigidBodyPosition.z - rGhostPosition.z
			};

			// ノーマライズして方向ベクトルに変更
			Direction.normalized();

			// 対象を最終的な方向へ跳ね飛ばす
			rpRigidBody->SetActive();
			rpRigidBody->SetImpulse(Direction * fPower);
			//rpRigidBody->SetLinearVelocity(Direction * fPower);
		}
	}
}

#pragma warning(pop)