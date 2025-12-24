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

// 物理挙動作成のため
#include "API.world.h"
#include "API.rigidbody.h"

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
{}

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
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::BOX, fWidth, fHeight, fDepth));

	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// Y軸以外の回転をロック
	pRB->SetAngularFactor({ 0.0f, 0.0f, 0.0f });

	// パラメータ参照
	const auto& param = m_ObstacleEditer.m_ParamSets[GetParamSetIndex()].subParams[GetSubParamIndex()];
	OBJ::Transform TF = {};

	TF.Pos = { param.ObstacleSpawnX, param.ObstacleSpawnY, param.ObstacleSpawnZ };

	// 位置セット
	pRB->SetWorldTransform(TF);
}

//============================================================================
// 更新処理
//============================================================================
void CTornado::Update()
{
	// 移動方向を設定
	SetMoveDir();

	// 物理オブジェクト用の更新：WVP行列用定数バッファの更新
	CPhysicsObject::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CTornado::Draw()
{
	// 物理オブジェクト用の描画：モデルの描画
	CPhysicsObject::Draw();
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

	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// リジッドボディのアクティブ化
	pRB->SetActive();

	// 設定用のトランスフォーム
	OBJ::Transform TF = pRB->GetWorldTransform();

	// 辺の移動ベクトルを計算
	DirectX::XMFLOAT3 delta = { EndPos.x - StartPos.x, 0.0f,EndPos.z - StartPos.z };

	// 辺の長さ計算
	float edgeLength = sqrt(delta.x * delta.x + delta.z * delta.z);

	// フレームごとの速度
	float speedPerFrame = 0.5f;						// 1フレームで進む距離
	m_edgeProgress += speedPerFrame / edgeLength;	// 現在の進行度を設定

	// 最大まで移動
	if (m_edgeProgress > 1.0f)
	{
		m_NowEdge = (m_NowEdge + 1) % NUM_EDGES;
		m_edgeProgress = 0.0f;
	}

	// 変更する位置に設定
	TF.Pos.x = StartPos.x + delta.x * m_edgeProgress;
	TF.Pos.z = StartPos.z + delta.z * m_edgeProgress;

	// ワールドトランスフォームに反映
	pRB->SetWorldTransform(TF);
}