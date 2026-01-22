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
#include <numbers> // C++20

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
		Window();
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
		auto sp = m_pwPlayer[nPlayerCount].lock();

		//削除されていたらスキップ
		if (!sp) continue;

		CRigidBody* pRB = DownCast<CRigidBody>(sp->GetCollider());
		if (!pRB) continue; //念のため

		ApplyWindToBody(pRB, Angle, speed);
	}

	// CPU
	for (int CPUCount = 0; CPUCount < CPUSize; ++CPUCount)
	{
		auto sp = m_pwEnemyPlayer[CPUCount].lock();

		//削除されていたらスキップ
		if (!sp) continue;

		CRigidBody* pRB = DownCast<CRigidBody>(sp->GetCollider());
		if (!pRB) continue;

		ApplyWindToBody(pRB, Angle, speed);
	}
}

//============================================================================
// 移動させる時の必要処理(まとめる用)そもそもあってるこのやり方？ああああああああ
//============================================================================
void CWindField::ApplyWindToBody(CRigidBody* pRB, float Angle, float speed)
{
	 btVector3 rCurrentVel = pRB->GetLinearVelocity();

	//加速値が無い時＝直前の加速値を仮代入 「いいやりかた欲しいね、、、、、」
	if (rCurrentVel == INIT)
	{
		rCurrentVel = m_SaverCurrentVel;
	}
	else
	{
		m_SaverCurrentVel = rCurrentVel;
	}

	pRB->SetActive();

	// 現在速度をコピー(値を変えるのでconstは×)
	btVector3 newVel = rCurrentVel;

	// 風の方向
	btVector3 windDir(sinf(Angle),0.0f,cosf(Angle));

	// 風を加算
	newVel += windDir * speed;

	// 風と逆方向に動いているか判定
	float dot = rCurrentVel.dot(windDir);

	if (dot < 0.0f)
	{
		// 抵抗力（逆方向のときだけ速度を弱める）
		float resistance = 0.5f; // 0.0～1.0（大きいほど抵抗が強い）
		newVel = newVel.lerp(windDir * speed, resistance);
	}

	// 移動方向：Y軸：現在の重力速度を維持
	newVel.setY(rCurrentVel.getY());

	//加速度の設定
	pRB->SetLinearVelocity(newVel+ rCurrentVel);
}


//============================================================================
//風のギミックの処理
//============================================================================
void CWindField::Window()
{
	++m_Parameter.m_Timer;

	if (m_Parameter.m_IsBlowing)
	{
		if (m_Parameter.m_Timer >= m_Parameter.m_BlowTime)
		{
			m_Parameter.m_Timer = 0.0f;
			m_Parameter.m_IsBlowing = false; // 風を止める
		}
		else
		{
			// プレイヤーとCPUを風で動かす
			MovePlayer(m_Parameter.m_WindAngle, m_Parameter.m_WindSpeed, PLAYER_SIZE, CPU_SIZE);
		}
	}
	else
	{
		// 風が止んでいる状態
		if (m_Parameter.m_Timer >= m_Parameter.m_StopTime)
		{
			m_Parameter.m_Timer = 0.0f;
			m_Parameter.m_IsBlowing = true; // 風を吹かせる

			// 新しい風向きをランダム生成
			m_Parameter.m_WindAngle = 3.14f;
			m_Parameter.m_WindSpeed = 1.0f;
		}
	}
}


//============================================================================
// 描画処理
//============================================================================
void CWindField::Draw()
{
	// 物理モデル用の更新
	CPhysicsModel::Draw();
}