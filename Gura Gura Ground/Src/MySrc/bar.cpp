//============================================================================
// 
// バー [bar.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "bar.h"

// 物理挙動作成のため
#include "API.world.h"
#include "API.rigidbody.h"

// エフェクト
#include "dust.h"
#include <obstacle_editer.h>

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

	// 高度
	float g_fAxisY_Spawn = 20.0f; // スポーン高度
	float g_fAxisY_Despawn = 3.0f;  // デスポーン高度

	// 位置表示
	void Print_Pos(const OBJ::Transform& TF)
	{
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		if (ImGui::TreeNodeEx("Bar", ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::Text("Bar Pos X: %.2f", TF.Pos.x);
			ImGui::Text("Bar Pos Y: %.2f", TF.Pos.y);
			ImGui::Text("Bar Pos Z: %.2f", TF.Pos.z);
			ImGui::TreePop();
		}
		ImGui::End();
	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CBar::CBar(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CObstacle(Type, Layer)
	, m_Direction(VEC3_ZERO_INIT)
{}

//============================================================================
// デストラクタ
//============================================================================
CBar::~CBar()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CBar::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// デフォルトのリジッドボディの生成
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::CYLINDER, fWidth, fHeight, fDepth));

	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// 質量を設定
	pRB->SetMass(1000.0f);

	// 出現
	Appear();
}

//============================================================================
// 更新処理
//============================================================================
void CBar::Update()
{
	// 挙動
	Action();

	//// 戻る
	//Loop();

	// 物理オブジェクト用の更新：WVP行列用定数バッファの更新
	CPhysicsObject::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CBar::Draw()
{
	// 物理オブジェクト用の描画：モデルの描画
	CPhysicsObject::Draw();
}

//============================================================================
// 出現
//============================================================================
void CBar::Appear()
{
	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(GetCollider());
	const auto& param = ObstacleEditer::s_ParamSets[m_ParamSetIndex].subParams[m_SubParamIndex];

	// 設定用のトランスフォーム
	OBJ::Transform TF = {};

	// 移動速度スケール作成
	const float fSpeed = 3.0f;

	TF.Pos = { param.ObstacleSpawnX, param.ObstacleSpawnY, param.ObstacleSpawnZ };
	SetDirection({ param.ObstacleSpeedX, param.ObstacleSpeedY, param.ObstacleSpeedZ });

	SetRotate(TF, GetDirection());

	// ワールドトランスフォームに反映
	pRigidBody->SetWorldTransform(TF);
}

//============================================================================
// 挙動
//============================================================================
void CBar::Action()
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
void CBar::Loop()
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

// 進行方向に応じて向きを変更
void CBar::SetRotate(OBJ::Transform& rTF, XMFLOAT3 Dir)
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