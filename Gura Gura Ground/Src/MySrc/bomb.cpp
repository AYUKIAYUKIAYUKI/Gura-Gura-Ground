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
#include "API.sound.manager.h"

// 物理モデル取得のため
#include "API.gltf.manager.h"

// 物理挙動作成のため
#include "API.rigidbody.h"

// エフェクト
#include "shadow.h"
#include "warning.h"
#include "effect.manager.h"

// 衝撃波の作成のため
#include "shockwave.h"
#include <obstacle_editer.h>

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CBomb::CBomb(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CObstacle(Type, Layer, Obstacle::OBSTACLE_TYPE::STATIONARY)
	, m_nTimer(0)
{
	// モデルのバインド
	SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Bomb"));

	// ピクセルシェーダーのバインド
	SetPixelShader(CPixelShaderManager::RefInstance().RefRegistry().BindAtKey("Ray.Marching"));
}

//============================================================================
// デストラクタ
//============================================================================
CBomb::~CBomb()
{
	if (CEffectManager::RefInstance().GetEffect(m_nEffHandle) != nullptr) {
		CEffectManager::RefInstance().GetEffect(m_nEffHandle)->SetDeath();
	}
}

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

	// パラメータ参照
	const auto& param = m_ObstacleEditer.m_ParamSets[GetParamSetIndex()].subParams[GetSubParamIndex()];
	OBJ::Transform TF = {};

	TF.Pos = { param.ObstacleSpawnX, param.ObstacleSpawnY, param.ObstacleSpawnZ };
	SetTimer(param.BombTimer);

	// 位置セット
	pRB->SetWorldTransform(TF);

	/* ！！！ 影の生成 ！！！ */
	CShadow* pShadow = CObjectManager::CreateRaw<CShadow>(OBJ::TYPE::NONE, OBJ::LAYER::DEFAULT);
	pShadow->SetTrackTarget(shared_from_this());

	/* ！！！ 警告表示の作成 ！！！ */
	CWarning* pWarning = CObjectManager::CreateRaw<CWarning>();
	std::shared_ptr<CObstacle> spObstacle = std::dynamic_pointer_cast<CObstacle>(shared_from_this());
	pWarning->SetTrackTarget(spObstacle);

	CEffect::Create(CEffectManager::TAG_SMOKE, TF.Pos, &m_nEffHandle, 0.1f* fDepth);
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
	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());
	useful::Vec3 pos = pRB->GetWorldTransform().Pos;
	pos.y += 1.4;
	if (CEffectManager::RefInstance().GetEffect(m_nEffHandle) != nullptr)CEffectManager::RefInstance().GetEffect(m_nEffHandle)->SetLocation(pos);

	// 障害物クラスの更新
	CObstacle::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CBomb::Draw()
{
	// 障害物クラスの描画
	CObstacle::Draw();
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
		// 効果音：爆弾
		CSoundManger::RefInstance().Play("Bomb", false, -0.5f, 0.6f);

		// コライダーをリジッドボディにキャスト
		const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());
		useful::Vec3 pos = pRB->GetWorldTransform().Pos;
		int eadf{};
		CEffect::Create(CEffectManager::TAG_BOMB, pos, &eadf, 1.4f);

		SetDeath();

		// 衝撃波の作成
		const float       fSpan = 3.0f;
		DirectX::XMFLOAT3 Size  = { fSpan, fSpan, fSpan };
		CreateShockWave(Collision::SHAPETYPE::SPHERE, Size, 30);
	}
}