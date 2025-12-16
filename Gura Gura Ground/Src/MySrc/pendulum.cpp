//============================================================================
// 
// 振り子 [pendulum.cpp]
// Author : 大竹熙
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "pendulum.h"

// 物理挙動作成のため
#include "API.world.h"
#include "API.rigidbody.h"

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

	// 高度
	float g_fAxisY_Spawn = 20.0f; // スポーン高度
	float g_fAxisY_Despawn = 3.0f;  // デスポーン高度

	// 振り子パラメータをまとめる
	namespace PendulumParams
	{
		const float Omega = 1.5f;					// 揺れる速さ
		const float Length = g_fFieldSpan * 1.2f;	// 振り子の長さ
		const float MaxAngle = 2.3f;				// 最大振れ角
		const float ShrinkY = 0.4f;					// 縦方向を潰す係数
		const float MarginY = 5.0f;					// 床から余裕分
	}

	// 位置表示
	void Print_Pos(const OBJ::Transform& TF)
	{
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		if (ImGui::TreeNodeEx("Pendulum", ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::Text("Pendulum Pos X: %.2f", TF.Pos.x);
			ImGui::Text("Pendulum Pos Y: %.2f", TF.Pos.y);
			ImGui::Text("Pendulum Pos Z: %.2f", TF.Pos.z);
			ImGui::TreePop();
		}
		ImGui::End();
	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CPendulum::CPendulum(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CObstacle(Type, Layer)
	, m_Direction(VEC3_ZERO_INIT)
	, m_Time(0.0f)
{}

//============================================================================
// デストラクタ
//============================================================================
CPendulum::~CPendulum()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CPendulum::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// デフォルトのリジッドボディの生成
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::SPHERE, fWidth, fHeight, fDepth));

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
void CPendulum::Update()
{
	const float dt = 1.0f / 60.0f; 
	m_Time += dt;

	// 挙動
	Action();

	// 戻る
	Loop();

	// 物理オブジェクト用の更新：WVP行列用定数バッファの更新
	CPhysicsObject::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CPendulum::Draw()
{
	// 物理オブジェクト用の描画：モデルの描画
	CPhysicsObject::Draw();
}

//============================================================================
// 出現
//============================================================================
void CPendulum::Appear()
{
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());
	OBJ::Transform TF = {};

	const float fSpeed = 3.0f;

	// 0: 奥→手前, 1: 手前→奥, 2: 右→左, 3: 左→右
	int nPattern = rand() % 4;

	switch (nPattern)
	{
	case 0: // 奥→手前
		SetDirection({ 0.0f, 0.0f, -fSpeed });
		m_Phase = DirectX::XM_PI * 0.5f;
		break;
	case 1: // 手前→奥
		SetDirection({ 0.0f, 0.0f, fSpeed });
		m_Phase = -DirectX::XM_PI * 0.5f;
		break;
	case 2: // 右→左
		SetDirection({ -fSpeed, 0.0f, 0.0f });
		m_Phase = DirectX::XM_PI * 0.5f;
		break;
	case 3: // 左→右
		SetDirection({ fSpeed, 0.0f, 0.0f });
		m_Phase = -DirectX::XM_PI * 0.5f;
		break;
	}

	// 支点は常に中央
	TF.Pos = { 0.0f, g_fAxisY_Spawn, 0.0f };

	m_Time = 0.0f;

	pRB->SetWorldTransform(TF);
}

//============================================================================
// 挙動
//============================================================================
void CPendulum::Action()
{
	// リジッドボディ（物理オブジェクト）の取得
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	const float theta = PendulumParams::MaxAngle * sinf(PendulumParams::Omega * m_Time + m_Phase);
	const float minY = g_fAxisY_Despawn + PendulumParams::MarginY;
	const float pivotY = minY + PendulumParams::Length * PendulumParams::ShrinkY;

	const float swing = PendulumParams::Length * sinf(theta);
	const float swingY = pivotY - (PendulumParams::Length * PendulumParams::ShrinkY) * cosf(theta);

	OBJ::Transform TF = {};

	if (m_Direction.z != 0.0f) 
	{
		// 奥⇄手前
		TF.Pos.x = 0.0f;
		TF.Pos.y = swingY;
		TF.Pos.z = swing;
	}
	else 
	{
		// 右⇄左
		TF.Pos.x = swing;
		TF.Pos.y = swingY;
		TF.Pos.z = 0.0f;
	}

	TF.Rot = DirectX::XMFLOAT4(0, 0, 0, 1);
	pRB->SetActive();
	pRB->SetWorldTransform(TF);
}

//============================================================================
// 戻る
//============================================================================
void CPendulum::Loop()
{
	// 1往復周期 = 2π / ω（端→端）
	const float period = 2.0f * DirectX::XM_PI / PendulumParams::Omega;

	// リジッドボディの取得
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// 現在のワールドトランスフォームを取得
	OBJ::Transform TF = {};
	pRB->GetWorldTransform(TF);

	if (m_Time >= period) 
	{
		m_Time = 0.0f;   // 時間リセット

		// 出現
		Appear();

		// 塵：拡散発生
		CDust::GenerateSpread(TF.Pos, 10);
	}

	/* 位置を出力*/
	Print_Pos(TF);
}