//================================================
//
//敵プレイヤーの処理(仮)[enemy.cpp]
//プレイヤーの処理を参考
//Auther:haruki chiba
//
//================================================


//================================================
//自身のインクルード
#include "enemy1.h"

//================================================
//前方宣言のインクルード
#include "player.h"
#include "shockwave.h"
#include "bar.h"

//================================================
//必要なインクルード
#include "API.object.manager.h" //オブジェクト情報を探すのに使用

//================================================
//名前空間（無名）
namespace
{
	//===================================================
	//マクロ定義
	btVector3 INIT = { 0.0f, 0.0f, 0.0f };   //btVector3用初期化マクロ

	const int MAX_RECASTTIME = 60;           //プレイヤーのリキャストタイムの最大値
	const int MAX_RECASTTIME_MOVE = 180;     //ゲーム開始時の移動までの時間
	const int MAX_RECASTTIME_IN_BAR = 60;    //バーのリキャストタイムの最大値

	//プレイヤーと同じ数値にする&&プレイヤーから直接同期->処理も変更しないと多分無理
	const float MOVE = 7.0f;                 //自身の移動値 (今はdebugで似てる速度を目視で設定中)
	const float JUMPPOWER = 13.5f;           //自身のジャンプ力
	const float DROPPOWER = JUMPPOWER * 1.5f;  //自身のドロップ速度
}

//================================================
//名前空間（使用）
using namespace DirectX;
using namespace useful;

#include "API.gltf.manager.h"

//======================================
//コンストラクタ
//======================================
CEnemyPlayer::CEnemyPlayer(OBJ::TYPE Type, OBJ::LAYER Layer) :
	CPhysicsModel(Type, Layer)  
, m_nRecasttime(0), m_bJump(true), m_pBar(nullptr)                                           
, m_pShockWave(nullptr), m_bGoDown(false), m_btOldVel(INIT)                    
, m_nStart(0), m_bStart(false)
{
	m_pPlayer.clear();
	searchPlayer();  //プレイヤーを探す(初めにプレイヤーが生成されてるのが条件)
	searchBar();     //障害物を探す(初めにプレイヤーが生成されてるのが条件)

	// モデルのバインド
	SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Test"));

	m_State = ENEMY_STATE::STATE_BASE;
}

//======================================
//デストラクタ
//======================================
CEnemyPlayer::~CEnemyPlayer()
{
	std::vector<CPlayer*>().swap(m_pPlayer);
	m_pBar = nullptr;
}


//============================================================================================================================================================================
//デフォルト(必須)処理群
//============================================================================================================================================================================

//============================================================================
// コライダーのファクトリ
//============================================================================
void CEnemyPlayer::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// 自身の用のリジッドボディの生成
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::BOX, fWidth, fHeight, fDepth));

	// コライダーをリジッドボディにキャスト
	CRigidBody* pRB = DownCast<CRigidBody>(GetCollider());

	// 重力の設定
	pRB->SetGravity({ 0.0f, -20.0f, 0.0f }); //プレイヤーより軽く設定(debug)

	// 摩擦力を設定
	pRB->SetFriction(1.0f);

	// Y軸以外の回転をロック
	pRB->SetAngularFactor(INIT);
}


//======================================
//更新処理
//======================================
void CEnemyPlayer::Update()
{
	//情報があるか確認
	CheckInfo();

	switch (m_State)
	{
	case ENEMY_STATE::STATE_BASE:    State_Base();    break;
	case ENEMY_STATE::STATE_IN_JUMP: State_In_Jump(); break;
	case ENEMY_STATE::STATE_BAR:     State_Bar();     break;
	}

	//基底クラスの更新
	CPhysicsModel::Update();
}


//============================================================================================================================================================================
//状態毎の処理群
//============================================================================================================================================================================

//======================================
//基本となる状態の処理
//======================================
void CEnemyPlayer::State_Base()
{
	//================================================================================================================================
	//プレイヤーの判定
	if (!m_pPlayer.empty())
	{
		std::vector<float>fvSaveDistance;  //各プレイヤーと自身の距離を算出
		std::vector<float>fvAngle;         //各プレイヤーと自身の向きを算出

		auto SelfPos = GetTransform().Pos; //自身の位置 

		//範囲baseでプレイヤーを参照
		for (const auto& pPlayer : m_pPlayer)
		{
			auto PlayerPos = pPlayer->GetTransform().Pos;                                //プレイヤーの位置

			fvAngle.push_back(atan2f(PlayerPos.x - SelfPos.x, PlayerPos.z - SelfPos.z)); //対角線の角度を算出する（目標ー自機）

			fvSaveDistance.push_back(CheckDistance(PlayerPos, SelfPos));                 //距離を算出
		}

		//-----------*全てのプレイヤーとの計算が終了＝算出したデータを比較し、最も近いプレイヤーを探す*-----------
		auto min_iterator = std::min_element(fvSaveDistance.begin(), fvSaveDistance.end()); //最小値取得
		size_t min_index = std::distance(fvSaveDistance.begin(), min_iterator);             //最小値を持つ番号取得

		auto PlayerPos = m_pPlayer[min_index]->GetTransform().Pos;   //プレイヤーの位置
		const float RADIUS = 4.0f;                                   //範囲

		//当たり判定
		if (CheckCollision(PlayerPos, SelfPos, RADIUS))
		{
			//ジャンプしてない
			if (m_bJump)
			{
				Jump_Base();

				ChangeState(ENEMY_STATE::STATE_IN_JUMP);
			}
		}
		else
		{
			//規定時間まで待ったかつジャンプを可能
			if (m_bJump&&m_bStart)
			{
				MoveAtPlayer(fvAngle[min_index], MOVE); //プレイヤーへ移動させる
			}

			//ここでゲーム開始時にすぐ動かないよう設定する
			else if(!m_bStart)
			{
				++m_nStart;
				
				//既定時間まで動かない
				if (m_nStart >= MAX_RECASTTIME_MOVE)
				{
					m_bStart = true;
				}
			}
		}
	}


	//================================================================================================================================
	//バーの判定
	if (m_pBar)
	{
		const float size = 1.0f; //当たり判定の大きさ

		//自身のトランスフォーム情報
		auto SelfTransform = GetTransform();
		XMFLOAT3 SelfSize = { size, size, size };
		GameObject self_GO = SetObbInfo(self_GO, SelfTransform.Pos, SelfSize, SelfTransform.Rot);

		//バーのトランスフォーム情報
		CRigidBody* pRB_Bar = DownCast<CRigidBody>(m_pBar->GetCollider());
		auto BarTransform = pRB_Bar->GetWorldTransform();
		XMFLOAT3 BarSize = { size, 15.0f, size };
		GameObject bar_GO = SetObbInfo(bar_GO, BarTransform.Pos, BarSize, BarTransform.Rot);

		if (self_GO.localOBB.Intersects(bar_GO.localOBB))
		{
			ChangeState(ENEMY_STATE::STATE_BAR);
		}
	}
}

//======================================
//飛んだ時の処理
//======================================
void CEnemyPlayer::State_In_Jump()
{
	//ジャンプした
	if (!m_bJump)
	{
		//疑似的に到達点を設定
		if (m_bGoDown)
		{
			//リジットボディを取得
			CRigidBody* pRB = DownCast<CRigidBody>(GetCollider());

			// ダイナミックに戻す
			pRB->SetDynamic();

			// アクティブに変更
			pRB->SetActive();

			// ドロップ力
			btVector3 btDropVec = { 0.0f, -DROPPOWER, 0.0f };

			// ドロップ力を反映
			pRB->SetImpulse(btDropVec);

			//CreateShockWave(Collision::SHAPETYPE::SPHERE, { 2.0f, 2.0f, 2.0f }, 1);
		}

		//リキャストタイムがMAXの設定値分到達した時にジャンプ出来るようにする
		if (InJump(m_bJump, m_nRecasttime, MAX_RECASTTIME))
		{
			ChangeState(ENEMY_STATE::STATE_BASE);
		}
	}
}

//======================================
//バーの処理
//======================================
void CEnemyPlayer::State_Bar()
{
	//ジャンプしてない
	if (m_bJump)
	{
		Jump_Base();
	}

	//ジャンプした
	if (!m_bJump)
	{
		//リキャストタイムがMAXの設定値分到達した時にジャンプ出来るようにする
		if (InJump(m_bJump, m_nRecasttime, MAX_RECASTTIME_IN_BAR))
		{
			ChangeState(ENEMY_STATE::STATE_BASE);
		}
	}
}


//============================================================================================================================================================================
//プレイヤーに関する処理群
//============================================================================================================================================================================

//======================================
//プレイヤーを探す処理
//======================================
void CEnemyPlayer::searchPlayer()
{
	//オブジェクトマネージャーのシェアポインターからプレイヤータイプを見つける
	auto  playerlist = CObjectManager::RefInstance().RefInstance().RefListShare(OBJ::TYPE::PLAYER);

	//範囲baseでプレイヤー情報の基盤を取得
	for (auto Obj : playerlist)
	{
		//キャストしてプレイヤーの情報を入れる
		CPlayer* pPlayer = dynamic_cast<CPlayer*>(Obj.get());
		m_pPlayer.push_back(pPlayer);
	}
}

//======================================
//プレイヤーの方へ移動する処理
//======================================
void CEnemyPlayer::MoveAtPlayer(float Angle, float speed)
{
	//リジットボディを取得
	CRigidBody* pRB = DownCast<CRigidBody>(GetCollider());

	//現在の加速度を参照
	const btVector3& rCurrentVel = pRB->GetLinearVelocity();

	//アクティブ化
	pRB->SetActive();

	//位置情報設定用
	btVector3   MoveDir = { INIT };

	//各位置の設定
	MoveDir.setX(sinf(Angle) * speed);
	MoveDir.setZ(cosf(Angle) * speed);

	// 移動方向：Y軸：現在の重力速度を維持
	MoveDir.setY(rCurrentVel.getY());

	pRB->SetLinearVelocity(MoveDir); //加速度の設定
}

//============================================================================
// 衝撃波の作成(未定)
//============================================================================
void CEnemyPlayer::CreateShockWave(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3A& Size, int nDuration)
{
	// 衝撃波の作成
	//m_pShockWave = CObject::Create<CShockWave>(OBJ::TYPE::NONE, OBJ::LAYER::DEFAULT);

	// プレイヤーの登録
	//m_pShockWave->SetPlayer(this);

	// プレイヤーのトランスフォームを出現位置に設定
	m_pShockWave->SetTransform(GetTransform());

	// ゴーストの作成
	m_pShockWave->FactoryCollider(Type, Size.x, Size.y, Size.z);

	// 衝撃波の作成
	m_pShockWave->SetDuration(nDuration);
}

//======================================
//当たり判定チェック処理
//======================================
bool CEnemyPlayer::CheckCollision(const XMFLOAT3& c1, const XMFLOAT3& c2, float Radius)
{
	//対角線を算出
	float centerDistance = CheckDistance(c1, c2);

	//中心点の距離より半径の和のほうが大きい
	if (centerDistance <= Radius)
	{
		return true; //二つの円が当たっている
	}

	return false;	 //二つの円が当たっていない
}

//======================================
//対角線の値を出す処理
//======================================
float CEnemyPlayer::CheckDistance(const XMFLOAT3& c1, const XMFLOAT3& c2)
{
	//各場所から値を算出
	float lengthX = c1.x - c2.x;
	float lengthY = c1.y - c2.y;
	float lengthZ = c1.z - c2.z;
	float centerDistance = sqrtf(lengthX * lengthX + lengthY * lengthY + lengthZ * lengthZ);

	return  centerDistance; //対角線の値を返す
}


//============================================================================================================================================================================
//バーに関する処理群
//============================================================================================================================================================================

//======================================
//バーを探す処理
//======================================
void CEnemyPlayer::searchBar()
{
	//オブジェクトマネージャーのシェアポインターからオブジェクトタイプを取得
	auto Obstaclelist = CObjectManager::RefInstance().RefInstance().RefListShare(OBJ::TYPE::OBSTACLE);

	//範囲baseで探す
	for (auto& Obj : Obstaclelist)
	{
		//Objの中身がcBarかどうかを判定(同じオブジェクトタイプでの判定)
		if (auto bar = std::dynamic_pointer_cast<CBar>(Obj))
		{
			//情報を取得し、回す必要がないので抜ける
			m_pBar = bar.get();
			break;
		}
	}
}

//======================================
//Obb情報を設定する処理
//======================================
CEnemyPlayer::GameObject& CEnemyPlayer::SetObbInfo(GameObject& Obj, const XMFLOAT3 pos, const XMFLOAT3 size, const XMFLOAT4 rot)
{
	//各パラメータを設定
	Obj.localOBB.Center = pos;
	Obj.localOBB.Extents = size;
	Obj.localOBB.Orientation = rot;

	return Obj;
}


//============================================================================================================================================================================
//共通する処理群
//============================================================================================================================================================================

//======================================
//ジャンプ中の処理(旧プレイヤーから参照)
//======================================
bool CEnemyPlayer::InJump(bool& bJump, int& RecastTme, const int MaxRecast)
{
	// リジッドボディの取得
	CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// 現在の加速度をコピー
	const btVector3& rCurrentVel = pRB->GetLinearVelocity();

	// 下降判定
	if (!m_bGoDown && rCurrentVel.getY() < 0.0f && m_btOldVel.getY() > 0.0f)
	{
		m_bGoDown = true; //下降ON
	}

	//下降中なにかに当たる時
	if (m_bGoDown && Collision::CheckHitToRigidBodyShare(pRB))
	{
		++RecastTme;

		//リキャストタイムが規定値に達した時
		if (RecastTme >= MaxRecast)
		{
			m_bGoDown = false;  //強制的に下降状態を解く（もう地上判定）
			RecastTme = 0;      //リキャストタイムを初期化
			bJump = true;       //ジャンプ可能

			return true;
		}
	}
	else if (m_bGoDown && Collision::CheckHitToRigidBodyRaw(pRB))
	{
		++RecastTme;

		//リキャストタイムが規定値に達した時
		if (RecastTme >= MaxRecast)
		{
			m_bGoDown = false;  //強制的に下降状態を解く（もう地上判定）
			RecastTme = 0;      //リキャストタイムを初期化
			bJump = true;       //ジャンプ可能

			return true;
		}
	}

	// 現在の加速度情報を次フレームへ持ち越し
	m_btOldVel = rCurrentVel;

	return false;
}

//======================================
//情報があるかどうか確認処理
//======================================
void CEnemyPlayer::CheckInfo()
{
	int nSize = (int)m_pPlayer.size() - 1; //プレイヤー数-1(０から数えるのため)

	//プレイヤーが死んだとき情報を消す
	for (int nCount = nSize; nCount >= 0; --nCount)
	{
		//死亡判定
		if (m_pPlayer[nCount]->GetDeath())
		{
			m_pPlayer.erase(m_pPlayer.begin() + nCount);
		}
	}

	//バーの情報を消す(死亡判定)
	if (m_pBar && m_pBar->GetDeath())
	{
		m_pBar = nullptr;
	}

	//======================================
	//自身の削除処理（プレイヤーと同じ条件）

	// コライダーをリジッドボディにキャスト
	CRigidBody* pRB = DownCast<CRigidBody>(GetCollider());

	// ワールドトランスフォームから位置を取得
	const DirectX::XMFLOAT3& Pos = pRB->GetWorldTransform().Pos;

	if (Pos.y < 3.0f)
	{
		// 自身の死亡フラグを立てる
		SetDeath();
	}
}

//======================================
//情報があるかどうか確認処理
//======================================
void CEnemyPlayer::Jump_Base()
{
	m_nRecasttime = 0; //リキャストタイムの初期化
	m_bJump = false;   //jump不可能

	//リジットボディを取得
	CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// ジャンプ力
	btVector3 btJumpVec = { 0.0f, JUMPPOWER, 0.0f };

	// ジャンプ力を衝撃として加える
	pRB->SetActive();
	pRB->SetImpulse(btJumpVec);
}

//======================================
//描画処理
//======================================
void CEnemyPlayer::Draw()
{
	CPhysicsModel::Draw();
}