//============================================================================
// 
// 風ステージの処理 [windfield.cpp]
// Author : 千葉
// 
//============================================================================

//****************************************************
// 自身のインクルード
//****************************************************
#include "windfield.h"

//****************************************************
// 自前方宣言のインクルード
//****************************************************
#include "player.h"
#include "enemy1.h"

//****************************************************
// 必要なインクルード
//****************************************************
#include "API.gltf.manager.h"

// コライダーの作成用
#include "API.rigidbody.h"

//================================================
//名前空間（無名）
namespace
{
	//===================================================
	//マクロ定義
	btVector3 INIT = { 0.0f, 0.0f, 0.0f };   //btVector3用初期化マクロ

	 int PLAYER_SIZE; //プレイヤーの人数
	 int CPU_SIZE;    //CPUの人数
}

//================================================
//使用名前空間
using namespace useful;


//============================================================================
// デフォルトコンストラクタ
//============================================================================
CWindField::CWindField(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CPhysicsModel(Type, Layer)
{
	// モデルのバインド
	SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Field"));

	// モデルオフセットの設定
	SetModelOffset({ 1.15f, 0.8f, -0.3f });
}

//============================================================================
// デストラクタ
//============================================================================
CWindField::~CWindField()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CWindField::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// フィールド用のリジッドボディの生成
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::BOX, fWidth, fHeight, fDepth));

	// コライダーをリジッドボディにキャスト
	CRigidBody* pRB = useful::DownCast<CRigidBody>(GetCollider());

	// スタティックに変更
	pRB->SetMass(0.0f);

	// 地面の反発係数を設定
	pRB->SetRestitution(1.0f);
}

//============================================================================
// 更新処理
//============================================================================
void CWindField::Update()
{
	UpdatePlayersSystem();

	// 物理モデル用の更新
	CPhysicsModel::Update();
}


//============================================================================
// 全てのプレイヤーシステムの更新処理
//============================================================================
void CWindField::UpdatePlayersSystem()
{
	//情報がないなら探す、あるなら処理
	if (m_pwPlayer.empty() && m_pwEnemyPlayer.empty())
	{
		SearchInfo(); //プレイヤーとCPUの情報を探す

		//必ず探した後で処理+何回もsize呼ぶのちーがうからあらかじめ取得
		PLAYER_SIZE = (int)m_pwPlayer.size();
		CPU_SIZE    = (int)m_pwEnemyPlayer.size();
	}
	else
	{
		MovePlayer(1.57f, 1.0f, PLAYER_SIZE, CPU_SIZE);
	}
}

//============================================================================
// 各情報を探す処理
//============================================================================
void CWindField::SearchInfo()
{
	//オブジェクトマネージャーのシェアポインターからプレイヤータイプを見つける
	const auto  playerlist = CObjectManager::RefInstance().RefInstance().RefListShare(OBJ::TYPE::PLAYER);
	const auto  enemyplayerlist = CObjectManager::RefInstance().RefInstance().RefListShare(OBJ::TYPE::NONE);

	//範囲baseでプレイヤー情報の基盤を取得
	for (auto Obj : playerlist)
	{
		//キャストしてプレイヤーの情報を入れる
		auto pPlayer = std::dynamic_pointer_cast<CPlayer>(Obj);
		m_pwPlayer.push_back(pPlayer);
	}

	//範囲baseで敵プレイヤー情報の基盤を取得
	for (auto Obj1 : enemyplayerlist)
	{
		//キャストしてCPUの情報を入れる
		auto pEnemyPlayer = std::dynamic_pointer_cast<CEnemyPlayer>(Obj1);
		m_pwEnemyPlayer.push_back(pEnemyPlayer);
	}
}

//============================================================================
// 移動させる処理 (向き、速度、プレイヤー人数,CPU人数)
//============================================================================
void CWindField::MovePlayer(float Angle, float speed, int PlayerSize, int CPUSize)
{
	// プレイヤー
	for (int nPlayerCount = 0; nPlayerCount < PlayerSize; ++nPlayerCount)
	{
		CRigidBody* pRB = DownCast<CRigidBody>(m_pwPlayer[nPlayerCount]->GetCollider());
		ApplyWindToBody(pRB, Angle, speed);
	}

	// CPU
	for (int CPUCount = 0; CPUCount < CPUSize; ++CPUCount)
	{
		CRigidBody* pRB = DownCast<CRigidBody>(m_pwEnemyPlayer[CPUCount]->GetCollider());
		ApplyWindToBody(pRB, Angle, speed);
	}
}

//============================================================================
// 移動させる時の必要処理(まとめる用)
//============================================================================
void CWindField::ApplyWindToBody(CRigidBody* pRB, float Angle, float speed)
{
	// 現在の加速度を参照
	const btVector3& rCurrentVel = pRB->GetLinearVelocity();

	// アクティブ化
	pRB->SetActive();

	// 移動方向
	btVector3 MoveDir(INIT);

	MoveDir.setX(sinf(Angle) * speed);
	MoveDir.setZ(cosf(Angle) * speed);

	// Y は重力の影響を維持
	MoveDir.setY(rCurrentVel.getY());

	// 速度を設定
	pRB->SetLinearVelocity(MoveDir);
}


//============================================================================
// 描画処理
//============================================================================
void CWindField::Draw()
{
	// 物理モデル用の更新
	CPhysicsModel::Draw();
}