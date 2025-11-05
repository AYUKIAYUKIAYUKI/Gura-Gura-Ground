//============================================================================
// 
// フィールド [filed.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "field.h"
#include "API.renderer.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CField::CField(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CAny(Type, Layer)
	, m_upAnyGeometry(nullptr)
	, m_upCollisionShape(nullptr)
	, m_upMotionState(nullptr)
	, m_upRigidBody(nullptr)
	, m_upConstraint(nullptr)
{}

//============================================================================
// デストラクタ
//============================================================================
CField::~CField()
{
	// ヒンジをワールドから削除
	if (m_upRigidBody)
	{
		CWorld::RefInstance().RefDynamicsWorldConst()->removeConstraint(m_upConstraint.get());
	}

	// 剛体をワールドから削除
	if (m_upRigidBody)
	{
		CWorld::RefInstance().RefDynamicsWorldConst()->removeRigidBody(m_upRigidBody.get());
	}
}

//============================================================================
// 更新処理
//============================================================================
void CField::Update()
{
	CAny::Update();

	// 描画用のトランスフォームの更新
	if (m_upRigidBody && m_upRigidBody->getMotionState())
	{
		// リジッドボディのトランスフォーム情報を取得
		btTransform Transform;
		m_upRigidBody->getMotionState()->getWorldTransform(Transform);

		// 向きと位置を取得
		const btQuaternion& Rotation = Transform.getRotation();
		const btVector3& Position = Transform.getOrigin();

		// 変換して上書き
		m_Transform.Rot = { Rotation.x(), Rotation.y(), Rotation.z(), Rotation.w() };
		m_Transform.Pos = { Position.x(), Position.y(), Position.z() };
	}
}

//============================================================================
// 描画処理
//============================================================================
void CField::Draw()
{
	CAny::Draw();

	// カメラを取得
	CCamera* pCamera = CRenderer::RefInstance().GetCamera();

	// VP行列を取得
	const DirectX::XMMATRIX& View = pCamera->GetView();
	const DirectX::XMMATRIX& Proj = pCamera->GetProjection();

	// トランスフォームからW行列を作成
	m_Transform.World =
		DirectX::XMMatrixScaling(m_Transform.Size.x, m_Transform.Size.y, m_Transform.Size.z) *
		DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(m_Transform.Rot.x, m_Transform.Rot.y, m_Transform.Rot.z, m_Transform.Rot.w)) *
		DirectX::XMMatrixTranslation(m_Transform.Pos.x, m_Transform.Pos.y, m_Transform.Pos.z);

	// 描画
	if (m_upAnyGeometry)
	{
		m_upAnyGeometry->Draw(
			m_Transform.World,
			View,
			Proj,
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			nullptr,
			false,
			nullptr);
	}
}