//============================================================================
// 
// 竜巻 [tornado.cpp]
// Author : 後藤優輝
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "tornado.h"
#include "API.gltf.manager.h"

// 物理挙動作成のため
#include "API.world.h"
#include "API.ghost.h"
#include "API.rigidbody.h"

#include "player.h"
#include "API.object.manager.h"
#include "API.sound.manager.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CTornado::CTornado(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CObstacle(Type, Layer, Obstacle::OBSTACLE_TYPE::PERIMETER)
	, m_StartPos(useful::VEC3_ZERO_INIT)
	, m_MoveDir(useful::VEC3_ZERO_INIT)
	, m_edgeProgress(0)
	, m_NowEdge(0)
	, m_Width(0.0f)
	, m_Depth(0.0f)
	, m_LapCount(1)
	, m_NowLapCount(0)
	, m_Life(10)
{
	// モデルのバインド
	SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Tornado"));
}

//============================================================================
// デストラクタ
//============================================================================
CTornado::~CTornado()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CTornado::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// 竜巻用のリジッドボディの作成
	SetCollider(CGhost::CreateGhost(GetTransform(), Collision::SHAPETYPE::BOX, fWidth, fHeight, fDepth));

	// コライダーをゴーストにキャスト
	const CGhost* const pGhost = dynamic_cast<CGhost*>(GetCollider());

	// パラメータ参照
	const auto& param = m_ObstacleEditer.m_ParamSets[GetParamSetIndex()].subParams[GetSubParamIndex()];
	OBJ::Transform TF = {};

	TF.Pos = { param.ObstacleSpawnX, param.ObstacleSpawnY, param.ObstacleSpawnZ };

	// 位置セット
	pGhost->SetWorldTransform(TF);

	// 効果音：風の音
	CSoundManger::RefInstance().Play("Tornado", false, -0.5f, 0.2f);
}

//============================================================================
// 更新処理
//============================================================================
void CTornado::Update()
{
	// 画面外に行くか判定
	if (IsOutOfScreen())
	{
		// 移動方向を設定
		SetMoveDir();

		// プレイヤーを引き寄せる
		PullPlayer();
	}
	else
	{
		// 画面外に移動
		MoveOutOfScreen();

		m_Life--;

		if (m_Life <= 0)
		{
			SetDeath();
		}
	}

	// 障害物クラスの更新
	CObstacle::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CTornado::Draw()
{
	// 障害物クラスの描画
	CObstacle::Draw();
}

//============================================================================
// パラメータの編集
//============================================================================
void CTornado::EditParam()
{

}

//============================================================================
// 移動方向を設定
//============================================================================
void CTornado::SetMoveDir()
{
	using namespace useful;
	constexpr int NUM_EDGES = 4;					// 辺の数
	constexpr float RATE = 0.05f;					// 辺の数
	DirectX::XMFLOAT3 TargetPositions[NUM_EDGES] =	// 目標地点
	{
		{m_StartPos.x,0.0f,m_StartPos.z},						// 左上
		{m_StartPos.x,0.0f,m_StartPos.z - m_Depth},				// 左下
		{m_StartPos.x + m_Width,0.0f,m_StartPos.z - m_Depth},	// 右下
		{m_StartPos.x + m_Width,0.0f,m_StartPos.z},				// 右上
	};

	DirectX::XMFLOAT3 StartPos = TargetPositions[m_NowEdge];				// 最初の位置
	DirectX::XMFLOAT3 EndPos = TargetPositions[(m_NowEdge + 1) % NUM_EDGES];// 最後の位置

	// コライダーをゴーストにキャスト
	const CGhost* const pGhost = dynamic_cast<CGhost*>(GetCollider());

	// 設定用のトランスフォーム
	OBJ::Transform TF = pGhost->GetWorldTransform();

	// 辺の移動ベクトルを計算
	m_MoveDir = { EndPos.x - StartPos.x, 0.0f,EndPos.z - StartPos.z };

	// 辺の長さ計算
	float edgeLength = sqrt(m_MoveDir.x * m_MoveDir.x + m_MoveDir.z * m_MoveDir.z);

	// フレームごとの速度
	float speedPerFrame = 0.1f;						// 1フレームで進む距離
	m_edgeProgress += speedPerFrame / edgeLength;	// 現在の進行度を設定

	// 変更する位置に設定
	TF.Pos.x = StartPos.x + m_MoveDir.x * m_edgeProgress;
	TF.Pos.z = StartPos.z + m_MoveDir.z * m_edgeProgress;

	// 最大まで移動
	if (m_edgeProgress > 1.0f)
	{
		int NextEdge = (m_NowEdge + 1) % NUM_EDGES;
		m_NowEdge = NextEdge;
		m_edgeProgress = 0.0f;

		if (NextEdge == 0)
		{
			m_NowLapCount++;
		}
	}

	// ワールドトランスフォームに反映
	pGhost->SetWorldTransform(TF);
}

//============================================================================
// プレイヤーを引き寄せる
//============================================================================
void CTornado::PullPlayer()
{
	// プレイヤーリスト取得
	auto PlayerList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);

	for (auto ite : PlayerList)
	{
		CPlayer* Player = dynamic_cast<CPlayer*>(ite.get());

		// リジッドボディの取得
		const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(Player->GetCollider());

		const btVector3& rCurrentVel = pRB->GetLinearVelocity();

		// 移動速度スケール
		const float fSpeed = 1.25f;
		btVector3   MoveDir = { 0.0f, 0.0f, 0.0f };

		DirectX::XMFLOAT3 TornadoPos = GetTransform().Pos;			// 竜巻の位置
		DirectX::XMFLOAT3 PlayerPos = Player->GetTransform().Pos;	// プレイヤーの位置
		float Dir = atan2f(TornadoPos.x - PlayerPos.x, TornadoPos.z - PlayerPos.z);// 向き

		// 移動方向：XZ軸：方向の単位ベクトルに速度を掛けたものを設定
		MoveDir.setX(sinf(Dir) * fSpeed);
		MoveDir.setZ(cosf(Dir) * fSpeed);
		MoveDir.setY(rCurrentVel.getY());

		// 線形速度を設定
		pRB->SetActive();
		pRB->SetLinearVelocity(rCurrentVel + MoveDir);
	}
}

//============================================================================
// 画面外に移動
//============================================================================
void CTornado::MoveOutOfScreen()
{
	// コライダーをゴーストにキャスト
	const CGhost* const pGhost = dynamic_cast<CGhost*>(GetCollider());

	// 速さ
	const float fSpeed = 0.02f;

	// 設定用のトランスフォーム
	OBJ::Transform TF = pGhost->GetWorldTransform();

	// 変更する位置に設定
	TF.Pos.x = TF.Pos.x + m_MoveDir.x * fSpeed;
	TF.Pos.z = TF.Pos.z + m_MoveDir.z * fSpeed;

	// 効果音：風の音
	CSoundManger::RefInstance().Stop("Tornado");

	// ワールドトランスフォームに反映
	pGhost->SetWorldTransform(TF);
}

//============================================================================
// 画面外に出るか判定
//============================================================================
bool CTornado::IsOutOfScreen()
{
	return m_NowLapCount < 1;
}