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
#include "API.object.manager.h"
#include "player.h"
#include "API.collision.h"

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
	float g_fAxisY_Despawn = 8.0f;  // デスポーン高度

	// その他設定値
	const int ViverateValue = 200;
	const DirectX::XMFLOAT3 SizeVec = {3.4f,4.0f,2.8f};

	// 位置表示
	void Print_Pos(const OBJ::Transform& TF)
	{
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		ImGui::Spacing();
		if (ImGui::TreeNodeEx("FallTetra", ImGuiTreeNodeFlags_OpenOnArrow))
		{
			if (ImGui::Button("Test Create"))
			{
				CObjectManager::CreateRaw<CFallTetra>(
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
	: CObstacle(Type, Layer, Obstacle::OBSTACLE_TYPE::STATIONARY)
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
	btVector3 Gravity = pRB->GetRigidBody()->getGravity();

	//重力を保存(FLOAT3とbtVector3の相互変換が多分無いので手動)
	m_InitalGravity.x = Gravity.x();	m_InitalGravity.y = Gravity.y();	m_InitalGravity.z = Gravity.z();

	// 質量を設定
	pRB->SetMass(1.0f);
	OBJ::Transform transform{};
	DirectX::XMFLOAT2 Vec2{};
	Vec2.x = SimpleUseful::GetRandomMT(-g_fFieldSpan, g_fFieldSpan);
	Vec2.y = SimpleUseful::GetRandomMT(-g_fFieldSpan, g_fFieldSpan);

	transform.Pos = { Vec2.x,g_fAxisY_Spawn,Vec2.y};
	//transform.Pos = { 0.0f,g_fAxisY_Spawn,0.0f };

	transform.Size = SizeVec;
	pRB->SetWorldTransform(transform);
	pRB->SetGravity({ 0.0f,0.0f,0.0f });
	m_InitalPosition = transform.Pos;
	ChangeState(std::make_shared<TetraState_Wait>(this));

}

//============================================================================
// 更新処理
//============================================================================
void CFallTetra::Update()
{
	// 挙動
	m_State->Action(this);

	//対プレイヤー判定
	ToSmash();

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
// 落ちてつぶす動き
//============================================================================
void CFallTetra::ToSmash()
{
	CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// オブジェクトのリストを取得
	const auto& rPlayerList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);

	for (const auto& e : rPlayerList)
	{
		std::shared_ptr<CPlayer> pPlayerObj = std::dynamic_pointer_cast<CPlayer>(e);

		// プレイヤー型にキャスト可能なら
		if (pPlayerObj)
		{
			CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(pPlayerObj->GetCollider());
			// 衝突判定
			Collision::MyContactCallbackRigidBodyAndRigidBody CallBack(pRB, pRigidBody);
			CWorld::RefInstance().RefDynamicsWorldConst()->contactPairTest(pRB->GetRigidBody(), pRigidBody->GetRigidBody(), CallBack);

			// 衝突が確認出来たら
			if (CallBack.m_bHit)
			{
				pPlayerObj->EnableFallTetraBehavior();
			}
		}
	}
}

//============================================================================
// ステートの切り替え
//============================================================================
void CFallTetra::ChangeState(std::shared_ptr<Tetra_State> NextState) {
	if (m_State == nullptr) { 
		m_State = std::make_shared<TetraState_Wait>(this); 
		return; 
	}
	m_State.reset();
	m_State = NextState;
}

//============================================================================
// 待機ステートの挙動
//============================================================================

TetraState_Wait::TetraState_Wait([[maybe_unused]] CFallTetra* p) :m_Timer(120), m_DefaultPos({0.0f,0.0f,0.0f})
{
	m_DefaultPos = p->GetInitalPosition();
}


void TetraState_Wait::Action([[maybe_unused]] CFallTetra* p)
{
	if (m_Timer < 0)
	{
		p->ChangeState(std::make_shared<TetraState_Fall>(p));
		return;
	}
	--m_Timer;
	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(p->GetCollider());

	//振動を加える
	OBJ::Transform transform{};
	DirectX::XMFLOAT2 Vec2{};
	Vec2.x = (SimpleUseful::GetRandomMT(-ViverateValue, ViverateValue)) * 0.001f;
	Vec2.y = (SimpleUseful::GetRandomMT(-ViverateValue, ViverateValue)) * 0.001f;

	transform.Pos = { m_DefaultPos.x + Vec2.x,g_fAxisY_Spawn,m_DefaultPos.z + Vec2.y };
	transform.Size = SizeVec;
	pRB->SetWorldTransform(transform);
}

TetraState_Fall::TetraState_Fall(CFallTetra* p) : m_Grace(20)
{
	// コライダーをリジッドボディにキャスト
	CRigidBody* const pRB = dynamic_cast<CRigidBody*>(p->GetCollider());
	DirectX::XMFLOAT3 gravity = { p->GetInitalGravity().x ,p->GetInitalGravity().y,p->GetInitalGravity().z };
	
	//重力を戻す
	pRB->SetGravity({ gravity.x,gravity.y,gravity.z});
}

void TetraState_Fall::Action([[maybe_unused]] CFallTetra* p)
{
	// コライダーをリジッドボディにキャスト
	CRigidBody* const pRB = dynamic_cast<CRigidBody*>(p->GetCollider());

	//下方向に移動量を加えてあげる
	pRB->GetRigidBody()->applyCentralForce({ 0.0f,-120.0f,0.0f });
	pRB->SetActive();

	//Y方向の移動量を取得
	btVector3 vel = pRB->GetRigidBody()->getLinearVelocity();
	btScalar velY = vel.getY();
	if (m_Grace > 0)
	{//落下する前に消えないように削除までの猶予をあげる
		--m_Grace;
		return;
	}
	// 現在のワールドトランスフォームを取得
	OBJ::Transform TF{};
	pRB->GetWorldTransform(TF);
	if (velY < 0.01f && velY > -0.01f)
	{//Y方向の移動量が一定値以下なら
		// 塵：拡散発生
		CDust::GenerateSpread(TF.Pos, 10);
		p->SetDeath();
	}
}