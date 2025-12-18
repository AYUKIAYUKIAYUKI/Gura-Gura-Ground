//============================================================================
// 
// ボム [bomb.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "bomb.h"
#include "API.object.manager.h"

// 物理挙動作成のため
#include "API.rigidbody.h"

// 衝撃波の作成のため
#include "shockwave.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CBomb::CBomb(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CObstacle(Type, Layer, Obstacle::OBSTACLE_TYPE::STATIONARY)
	, m_nTimer(0)
{}

//============================================================================
// デストラクタ
//============================================================================
CBomb::~CBomb()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CBomb::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// ボール用のリジッドボディの生成
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::SPHERE, fWidth, fHeight, fDepth));

	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// 質量を設定
	pRB->SetRestitution(10.0f);

	// 回転摩擦力を設定
	pRB->SetRollingFriction(5.0f);

	// 弾性力を設定
	pRB->SetRestitution(0.25f);
}

//============================================================================
// 衝撃波の作成
//============================================================================
void CBomb::CreateShockWave(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3& Size, int nDuration)
{
	// 衝撃波の作成と、弱参照の設定
	const std::shared_ptr<CShockWave>& spShockWave = CObjectManager::CreateShare<CShockWave>(
		OBJ::TYPE::NONE,
		OBJ::LAYER::DEFAULT);

	// 自身のトランスフォームを出現位置に設定
	spShockWave->SetTransform(GetTransform());

	// ゴーストの作成
	spShockWave->FactoryCollider(Type, Size.x, Size.y, Size.z);

	// 自身を無視対象に設定
	spShockWave->SetIgnore(shared_from_this());

	// 期間の設定
	spShockWave->SetDuration(nDuration);
}

//============================================================================
// 更新処理
//============================================================================
void CBomb::Update()
{
	// 挙動
	Action();

	// 物理オブジェクト用の更新：WVP行列用定数バッファの更新
	CPhysicsObject::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CBomb::Draw()
{
	// 物理オブジェクト用の描画：モデルの描画
	CPhysicsObject::Draw();
}

//============================================================================
// インスペクターの表示
//============================================================================
void CBomb::ShowInspector()
{
	// 爆弾のパラメータ出力
	useful::MIS::MyImGuiShortcut_BeginWindow("Bomb Param");
	ImGui::Text("Direction X: %.2f", GetTransform().Pos.x);
	ImGui::Text("Direction Y: %.2f", GetTransform().Pos.y);
	ImGui::Text("Direction Z: %.2f", GetTransform().Pos.z);
	ImGui::End();
}

//============================================================================
// パラメータの編集
//============================================================================
void CBomb::EditParam()
{}

//============================================================================
// 挙動
//============================================================================
void CBomb::Action()
{
	--m_nTimer;

	if (m_nTimer <= 0)
	{
		SetDeath();

		// 衝撃波の作成
		const float       fSpan = 10.0f;
		DirectX::XMFLOAT3 Size  = { fSpan, fSpan, fSpan };
		CreateShockWave(Collision::SHAPETYPE::CYLINDER, Size, 30);
	}
}