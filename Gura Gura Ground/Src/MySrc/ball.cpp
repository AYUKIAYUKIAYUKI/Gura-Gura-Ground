//============================================================================
// 
// ボール [ball.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "ball.h"

// 物理挙動作成のため
#include "API.world.h"
#include "API.rigidbody.h"

// エフェクト
#include "dust.h"
#include <obstacle_editer.h>

//****************************************************
// 無名名前空間の定義
//****************************************************
namespace
{
	// フィールドサイズ
	float g_fFieldSpan = 15.0f;
	float g_fFieldHalf = g_fFieldSpan * 0.5f;

	// 高度
	float g_fAxisY_AirPow  = 0.1f;  // 浮力
	float g_fAxisY_Jump    = 3.0f;  // ジャンプ力
	float g_fAxisY_Spawn   = 10.0f; // スポーン高度
	float g_fAxisY_Despawn = 2.0f;  // デスポーン高度

	// 数値操作用
	void ValueControl()
	{
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		ImGui::DragFloat("Air Pow", &g_fAxisY_AirPow, 0.01f);
		ImGui::DragFloat("Jump", &g_fAxisY_Jump, 0.01f);
		ImGui::DragFloat("Spawn", &g_fAxisY_Spawn, 0.01f);
		ImGui::DragFloat("Despawn", &g_fAxisY_Despawn, 0.01f);
		ImGui::End();
	}

	// 位置表示
	void Print_Pos(const OBJ::Transform& TF)
	{
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		if (ImGui::TreeNodeEx("Ball", ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::Text("Ball Pos X: %.2f", TF.Pos.x);
			ImGui::Text("Ball Pos Y: %.2f", TF.Pos.y);
			ImGui::Text("Ball Pos Z: %.2f", TF.Pos.z);
			ImGui::TreePop();
		}
		ImGui::End();
	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CBall::CBall(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CObstacle(Type, Layer, Obstacle::OBSTACLE_TYPE::MOVING)
	, m_Direction(useful::VEC3_ZERO_INIT)
{}

//============================================================================
// デストラクタ
//============================================================================
CBall::~CBall()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CBall::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// ボール用のリジッドボディの生成
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::SPHERE, fWidth, fHeight, fDepth));

	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// 弾性力を設定
	pRB->SetRestitution(1.0f);

	// 出現
	Appear();
}

//============================================================================
// 更新処理
//============================================================================
void CBall::Update()
{
	// 挙動
	Action();

	//// 戻る
	//Loop();

	// 物理オブジェクト用の更新：WVP行列用定数バッファの更新
	CPhysicsObject::Update();

	// ImGui
	ValueControl();
}

//============================================================================
// 描画処理
//============================================================================
void CBall::Draw()
{
	// 物理オブジェクト用の描画：モデルの描画
	CPhysicsObject::Draw();
}

//============================================================================
// インスペクターの表示
//============================================================================
void CBall::ShowInspector()
{
	// ボールのパラメータ出力
	useful::MIS::MyImGuiShortcut_BeginWindow("Ball Param");
	ImGui::Text("Direction X: %.2f", m_Direction.x);
	ImGui::Text("Direction Y: %.2f", m_Direction.y);
	ImGui::Text("Direction Z: %.2f", m_Direction.z);
	ImGui::End();
}

//============================================================================
// パラメータの編集
//============================================================================
void CBall::EditParam()
{}

//============================================================================
// 出現
//============================================================================
void CBall::Appear()
{
	const auto& param = ObstacleEditer::s_ParamSets[m_ParamSetIndex].subParams[m_SubParamIndex];
	const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(GetCollider());
	OBJ::Transform TF = {};

	TF.Pos = { param.ObstacleSpawnX, param.ObstacleSpawnY, param.ObstacleSpawnZ };
	SetDirection({ param.ObstacleSpeedX, param.ObstacleSpeedY, param.ObstacleSpeedZ });

	// ワールドトランスフォームに反映
	pRigidBody->SetWorldTransform(TF);
}

//============================================================================
// 挙動
//============================================================================
void CBall::Action()
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
}

//============================================================================
// 戻る
//============================================================================
void CBall::Loop()
{
	// リジッドボディの取得
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// 現在のワールドトランスフォームを取得
	OBJ::Transform TF = {};
	pRB->GetWorldTransform(TF);

	/* 場外に出たら：Y軸の位置がフィールド位置を下回ったら */
	if (TF.Pos.y < g_fAxisY_Despawn)
	{
		// 初速をリセット
		btVector3 MoveDir = { 0.0f, 0.0f, 0.0f };
		pRB->SetLinearVelocity(MoveDir);

		// 出現
		Appear();

		// 塵：拡散発生
		CDust::GenerateSpread(TF.Pos, 10);
	}

	/* 位置を出力*/
	Print_Pos(TF);
}