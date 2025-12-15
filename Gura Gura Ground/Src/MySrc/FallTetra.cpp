//============================================================================
// 
// ドッスン [FallTetra.cpp]
// Author : 元地弘汰
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "FallTetra.h"
#include <random>

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
	float g_fAxisY_Spawn = 15.0f; // スポーン高度
	float g_fAxisY_Despawn = 3.0f;  // デスポーン高度

	// その他設定値
	const int ViverateValue = 200;
	const DirectX::XMFLOAT3 SizeVec = {2.8f,3.4f,2.4f};

	// 位置表示
	void Print_Pos(const OBJ::Transform& TF)
	{
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		ImGui::Spacing();
		if (ImGui::TreeNodeEx("FallTetra", ImGuiTreeNodeFlags_OpenOnArrow))
		{
			if (ImGui::Button("Test Create"))
			{
				CObject::Create<CFallTetra>(
					[](CFallTetra* p) ->bool
					{
						p->FactoryCollider();
						return true;
					},
					OBJ::TYPE::OBSTACLE);
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}
	namespace SimpleUseful {
		int GetRandomMT(int min, int max) {
			std::random_device rnd;				// 非決定的な乱数生成器でシード生成機を生成
			std::mt19937 mt(rnd());				//  メルセンヌツイスターの32ビット版、引数は初期シード
			std::uniform_int_distribution<> rand_num(min, max);     // 指定範囲無いの一様乱数
			return (rand_num(mt));
		}
	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CFallTetra::CFallTetra(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CObstacle(Type, Layer)
{}

//============================================================================
// デストラクタ
//============================================================================
CFallTetra::~CFallTetra()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CFallTetra::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// デフォルトのリジッドボディの生成
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::BOX, SizeVec.x, SizeVec.y, SizeVec.z));
		
	// コライダーをリジッドボディにキャスト
	CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// 質量を設定
	pRB->SetMass(1.0f);
	OBJ::Transform transform{};
	DirectX::XMFLOAT2 Vec2;
	Vec2.x = SimpleUseful::GetRandomMT(-g_fFieldSpan, g_fFieldSpan);
	Vec2.y = SimpleUseful::GetRandomMT(-g_fFieldSpan, g_fFieldSpan);

	transform.Pos = { Vec2.x,g_fAxisY_Spawn,Vec2.y};
	transform.Size = SizeVec;
	pRB->SetWorldTransform(transform);
	pRB->SetGravity({ 0.0f,0.0f,0.0f });

	// 出現
	Appear();
}

//============================================================================
// 更新処理
//============================================================================
void CFallTetra::Update()
{
	// 挙動
	m_State->Action(this);

	//// 戻る
	//Loop();

	// 物理オブジェクト用の更新：WVP行列用定数バッファの更新
	CPhysicsObject::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CFallTetra::Draw()
{
	// 物理オブジェクト用の描画：モデルの描画
	CPhysicsObject::Draw();
}


//============================================================================
// 待機ステートの挙動
//============================================================================

void TetraState_Wait::Action([[maybe_unused]] CFallTetra* p)
{
	if (m_Timer < 0)
	{
		p->ChangeState(std::make_shared<TetraState_Fall>(this));
		return;
	}
	++m_Timer;
	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(p->GetCollider());

	// 質量を設定
	pRB->SetMass(1.0f);
	OBJ::Transform transform{};
	DirectX::XMFLOAT2 Vec2;
	Vec2.x = (SimpleUseful::GetRandomMT(-ViverateValue, ViverateValue)) * 0.01f;
	Vec2.y = (SimpleUseful::GetRandomMT(-ViverateValue, ViverateValue)) * 0.01f;

	transform.Pos = { Vec2.x,ViverateValue,Vec2.y };
	transform.Size = SizeVec;
	pRB->SetWorldTransform(transform);
}

TetraState_Fall::TetraState_Fall(CFallTetra* p)
{
	// コライダーをリジッドボディにキャスト
	CRigidBody* const pRB = dynamic_cast<CRigidBody*>(p->GetCollider());
	pRB->SetGravity({ 0.0f,-6.0f,0.0f });
}

void TetraState_Fall::Action([[maybe_unused]] CFallTetra* p)
{
	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(p->GetCollider());
	if (pRB->GetLinearVelocity().getY() <= 0)
	{
		// 現在のワールドトランスフォームを取得
		OBJ::Transform TF = {};
		pRB->GetWorldTransform(TF);
		// 塵：拡散発生
		CDust::GenerateSpread(TF.Pos, 10);
		p->SetDeath();
	}
}