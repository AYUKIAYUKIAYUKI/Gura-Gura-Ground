//============================================================================
// 
// 塵 [dust.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "dust.h"
#include "API.gltf.manager.h"
#include "API.rigidbody.h"
#include "API.world.h"

// オブジェクト生成のため
#include "API.object.manager.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CDust::CDust(OBJ::TYPE Type, OBJ::LAYER Layer)
	 : CPhysicsModel(Type, Layer)
{
	// モデルのバインド
	SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Dust"));
}

//============================================================================
// デストラクタ
//============================================================================
CDust::~CDust()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CDust::FactoryCollider()
{
	// 軸方向のスパン
	const float fSpan = RB_SPAN;

	// ボール用のリジッドボディの生成
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::BOX, fSpan, fSpan, fSpan));

	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// 質量を設定
	pRB->SetMass(0.1f);

	// 弾性力を設定
	pRB->SetRestitution(0.35f);

	// 摩擦力を設定
	pRB->SetFriction(0.8f);
}

//============================================================================
// 更新処理
//============================================================================
void CDust::Update()
{
	// 非アクティブ状態になったら、死亡フラグを立てる
	if (!useful::DownCast<CRigidBody>(GetCollider())->GetActive())
	{
		SetDeath();
	}

	// 物理オブジェクト用の更新処理
	CPhysicsModel::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CDust::Draw()
{
	// 物理オブジェクト用の描画処理
	CPhysicsModel::Draw();
}

//============================================================================
// 拡散発生
//============================================================================
void CDust::GenerateSpread(const DirectX::XMFLOAT3& Pos, int nNum)
{
	for (int nGenerateNum = 0; nGenerateNum < nNum; ++nGenerateNum)
	{
		CObjectManager::CreateRaw<CDust>([&Pos](CDust* pObj) -> bool
			{
				const DirectX::XMFLOAT3 GeneratePos =
				{
					Pos.x + useful::GetRandomValue<float>() * 0.005f,
					Pos.y,
					Pos.z + useful::GetRandomValue<float>() * 0.005f
				};

				OBJ::Transform TF = {};
				TF.Size = TF_SPAN;
				TF.Pos = GeneratePos;
				pObj->SetTransform(TF);

				pObj->FactoryCollider();

				CRigidBody* pRB = useful::DownCast<CRigidBody>(pObj->GetCollider());

				const btVector3 Dir =
				{	
					GeneratePos.x - Pos.x,
					GeneratePos.y - Pos.y,
					GeneratePos.z - Pos.z
				};

				pRB->SetImpulse(Dir);
				pRB->SetTorqueImpulse(Dir * 0.05f);

				return true;
			});
	}
}

//============================================================================
// 直線発生
//============================================================================
void CDust::GenerateLinear(const DirectX::XMFLOAT3& Pos, const DirectX::XMFLOAT3& Dir)
{
	CObjectManager::CreateRaw<CDust>([&Pos, &Dir](CDust* pObj) -> bool
		{
			const DirectX::XMFLOAT3 GeneratePos =
			{
				Pos.x,
				Pos.y,
				Pos.z
			};

			OBJ::Transform TF = {};
			TF.Size = TF_SPAN;
			TF.Pos = GeneratePos;
			pObj->SetTransform(TF);

			pObj->FactoryCollider();

			CRigidBody* pRB = useful::DownCast<CRigidBody>(pObj->GetCollider());

			btVector3 ResDir = 
			{
				Dir.x,
				Dir.y,
				Dir.z
			};

			pRB->SetImpulse(ResDir * 0.01f);
			pRB->SetTorqueImpulse(ResDir * 0.01f);

			return true;
		});
}