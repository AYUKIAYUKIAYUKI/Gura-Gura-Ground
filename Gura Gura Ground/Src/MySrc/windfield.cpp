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
	if (m_pwPlayer.empty() && m_pwEnemyPlayer.empty())
	{
		SearchInfo();
	}

	// 物理モデル用の更新
	CPhysicsModel::Update();
}

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


		//範囲baseで敵プレイヤー情報の基盤を取得
		for (auto Obj1 : enemyplayerlist)
		{
			//キャストしてプレイヤーの情報を入れる
			auto pEnemyPlayer = std::dynamic_pointer_cast<CEnemyPlayer>(Obj1);
			m_pwEnemyPlayer.push_back(pEnemyPlayer);
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