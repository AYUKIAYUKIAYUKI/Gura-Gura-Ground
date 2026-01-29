//============================================================================
// 
// 氷フィールド [field_ice.cpp]
// Author : 大竹熙
// 
//============================================================================

#include "field.ice.h"
#include "API.gltf.manager.h"
#include "API.rigidbody.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CFieldIce::CFieldIce(OBJ::TYPE Type, OBJ::LAYER Layer)
    : CField(Type, Layer)
{
    // フィールドタイプ設定
    m_FieldType = FIELD_TYPE::ICE;

    // 氷ステージ用モデル
    SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("IceField"));

    // モデルオフセット
    SetModelOffset({ 1.15f, -5.5f, 0.0f });
}

//============================================================================
// デストラクタ
//============================================================================
CFieldIce::~CFieldIce()
{
}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CFieldIce::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
    // 氷フィールド用のリジッドボディ生成
    SetCollider(CRigidBody::CreateRigidBody(
        GetTransform(),
        Collision::SHAPETYPE::BOX,
        fWidth, fHeight, fDepth
    ));

    CRigidBody* pRB = useful::DownCast<CRigidBody>(GetCollider());

    // 静的化
    pRB->SetMass(0.0f);

    // 反発係数
    pRB->SetRestitution(1.0f);

    // 氷の摩擦設定
    pRB->SetFriction(0.0f);
}

//============================================================================
// 更新処理
//============================================================================
void CFieldIce::Update()
{
    // CField（CPhysicsModel）の更新処理をそのまま使う
    CField::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CFieldIce::Draw()
{
    // CField（CPhysicsModel）の描画処理をそのまま使う
    CField::Draw();
}