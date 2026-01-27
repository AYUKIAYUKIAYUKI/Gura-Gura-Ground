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
#include <numbers> 
#include "API.input.manager.h"

//================================================
//名前空間（無名）
namespace
{
	//===================================================
	//マクロ定義
	btVector3 INIT = { 0.0f, 0.0f, 0.0f };                    //btVector3用初期化マクロ
	btVector3 INIT_PLYER_VELOCITY = { 0.0f, 1.144f, 0.0f };   //ゲーム開始時のプレイヤーの初期加速値を疑似的に設定
	const float ANGLE = (float)std::numbers::pi*0.5f;         //右から始まるよ
	const float AIR_SPEED = 0.55f;                            //空中時の風の強さ調整値(ある程度の速度を残しつつ即死を防ぐ)

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
	, m_WindowRotationAngle(ANGLE), m_SaverCurrentVel(INIT_PLYER_VELOCITY)
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
	if (m_pwPlayer.empty())
	{
		SearchPYInfo(); //プレイヤーとCPUの情報を探す
		PLAYER_SIZE = (int)m_pwPlayer.size();
	}
	else if (m_pwEnemyPlayer.empty())
	{
		SearchCPUInfo(); //プレイヤーとCPUの情報を探す
		CPU_SIZE = (int)m_pwEnemyPlayer.size();
	}
	
	//上記が揃ってから起動＝むらを無くせるし、安全
	else
	{
		Window();
	}
}

//============================================================================
// 各情報を探す処理
//============================================================================
void CWindField::SearchPYInfo()
{
	//オブジェクトマネージャーのシェアポインターからプレイヤータイプを見つける
	const auto  playerlist = CObjectManager::RefInstance().RefInstance().RefListShare(OBJ::TYPE::PLAYER);

	//範囲baseでプレイヤー情報の基盤を取得
	for (auto Obj : playerlist)
	{
		//キャストしてプレイヤーの情報を入れる
		auto pPlayer = std::dynamic_pointer_cast<CPlayer>(Obj);
		m_pwPlayer.push_back(pPlayer);
	}
}

//============================================================================
// 各情報を探す処理
//============================================================================
void CWindField::SearchCPUInfo()
{
	//オブジェクトマネージャーのシェアポインターからプレイヤータイプを見つける
	const auto  enemyplayerlist = CObjectManager::RefInstance().RefInstance().RefListShare(OBJ::TYPE::CPU);

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

		ApplyWindToBody_PY(pRB, Angle, speed, m_pwPlayer[nPlayerCount]);
	}

	// CPU
	for (int CPUCount = 0; CPUCount < CPUSize; ++CPUCount)
	{
		auto CUP = m_pwEnemyPlayer[CPUCount].lock();

		//削除されていたらスキップ
		if (!CUP) continue;

		CRigidBody* pRB = DownCast<CRigidBody>(CUP->GetCollider());
		if (!pRB) continue;

		ApplyWindToBody_CPU(pRB, Angle, speed, m_pwEnemyPlayer[CPUCount]); // CPUは補正なし
	}
}

//============================================================================
// 移動させる時の必要処理(まとめる用)プレイヤー用
//============================================================================
void CWindField::ApplyWindToBody_PY(CRigidBody* pRB, float Angle, float speed, std::weak_ptr<CPlayer > pwPlayer)
{
	const auto& opDirection =
		CInputManager::RefInstance().ConvertInputToMoveDirection(pwPlayer.lock()->GetIdxPlayer());

	//空中にいる時
	if (!CheckLand(pwPlayer))
	{
		speed = speed * AIR_SPEED;
	}

	//入力時、強める
	if (opDirection)
	{
		speed = speed * 1.15f;
	}

	ApplyWindCommon(pRB, Angle, speed);
}


//============================================================================
// 移動させる時の必要処理(まとめる用)CPU用
//============================================================================
void CWindField::ApplyWindToBody_CPU(CRigidBody* pRB, float Angle, float speed, std::weak_ptr<CEnemyPlayer> pwCPU)
{
	//空中にいる時
	if (!CheckLand(pwCPU))
	{
		speed = speed * AIR_SPEED;
	}

	ApplyWindCommon(pRB, Angle, speed);
}

//============================================================================
//風の影響を適用する
//============================================================================
void CWindField::ApplyWindCommon(CRigidBody* pRB, float Angle, float speed)
{
	btVector3 rCurrentVel = pRB->GetLinearVelocity();

	//加速値がない（移動してない）
	if (rCurrentVel == INIT)
	{
		rCurrentVel = m_SaverCurrentVel;
	}

	pRB->SetActive();

	btVector3 newVel = rCurrentVel;

	// 風方向
	btVector3 windDir(sinf(Angle), 0.0f, cosf(Angle));

	// 風を加算
	newVel += (windDir * speed);

	// 逆方向なら抵抗
	float dot = rCurrentVel.dot(windDir);
	if (dot < 0.0f) {
		float resistance = 0.5f;
		newVel = newVel.lerp(windDir * speed, resistance);
	}

	// Y軸は重力速度を維持
	newVel.setY(rCurrentVel.getY());

	// 最終速度を設定
	pRB->SetLinearVelocity(newVel);
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
			m_Parameter.m_Timer = 0.0f;     //タイマーリセット
			m_Parameter.m_IsBlowing = true; //風を吹かせる

			//m_Parameter.m_WindAngle = RandomRange(-(float)std::numbers::pi, (float)std::numbers::pi); //風の向き
			m_Parameter.m_WindAngle = m_WindowRotationAngle; //風の向き
			m_Parameter.m_WindSpeed = 0.9f;                  //風の強さ

			//念のため一周したら初期化(右回り時のif分)
			if (m_WindowRotationAngle >= ANGLE * 4.0f)
			{
				m_WindowRotationAngle = 0.0f;
			}
			m_WindowRotationAngle += ANGLE; //向きを加算
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