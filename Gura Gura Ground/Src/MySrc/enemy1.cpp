//================================================
//
//敵の処理(仮)[enemy1.cpp]
//プレイヤーの処理を参考+極力他のcppを変更しないように処理
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

//================================================
//必要なインクルード
#include <API.gltf.manager.h>
#include <any.h>
#include "API.object.manager.h"


//======================================
//コンストラクタ
//======================================
CEnemy1::CEnemy1(OBJ::TYPE Type, OBJ::LAYER Layer) :CPhysicsObject(Type, Layer),
m_nRecasttime(MAX_RECASTTIME_IN), m_bJump(true), m_bTop(false)
{
	searchPlayer();  //プレイヤーを探す(初めにプレイヤーが生成されてるのが条件)
}

//======================================
//デストラクタ
//======================================
CEnemy1::~CEnemy1()
{
	std::vector<CPlayer*>().swap(m_pPlayer); //解放処理(clear関数だけではデストラクタが呼ばれず？メモリ解放ができない)
}

//======================================
//初期化処理
//======================================
bool CEnemy1::Initialize()
{
	return true;
}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CEnemy1::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// 自身の用のリジッドボディの生成
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::BOX, fWidth, fHeight, fDepth));

	//// コライダーをリジッドボディにキャスト
	CRigidBody* pRB =DownCast<CRigidBody>(GetCollider());

	// Y軸以外の回転をロック
	pRB->SetAngularFactor({ 0.0f, 0.0f, 0.0f });
	
}


//======================================
//プレイヤーを探す処理(これも改善点かも)
//======================================
void CEnemy1::searchPlayer()
{
	//オブジェクトmanagerからプレイヤータイプを見つける
	std::list<CObject*> playerlist = CObjectManager::RefInstance().RefObjList(OBJ::TYPE::PLAYER);

	//範囲baseでプレイヤー情報の基盤を取得
	for (const auto Obj : playerlist)
	{
		m_pPlayer.push_back(static_cast<CPlayer*>(Obj)); //プレイヤーの情報を入れる
	}
}


//======================================
//更新処理
//======================================
void CEnemy1::Update()
{
	DeletePlayerInfo(); //前持ってプレイヤーが生きてるか判断する(いちいち処理を呼ぶのはhmm,,,)

	//プレイヤーの情報がある時
	if (!m_pPlayer.empty())
	{
		Calculation();  //計算処理
	}

	CPhysicsObject::Update();

	DeleteSelf();       //自身を消す
}

//======================================
//プレイヤーの情報を消す ->ゆうきのやつがいいかも（TYPEを追加しないと無理）
//======================================
void CEnemy1::DeletePlayerInfo()
{
	int nSize = (int)m_pPlayer.size() - 1;

	//プレイヤーが死んだとき情報を消す(後ろから消す)
	for (int nCount = nSize; nCount >= 0; --nCount)
	{
		if (m_pPlayer[nCount]->GetDeath())
		{
			m_pPlayer.erase(m_pPlayer.begin() + nCount);
		}
	}
}

//======================================
//自身を消す処理（プレイヤーと同じ条件）
//======================================
void CEnemy1::DeleteSelf()
{
	// コライダーをリジッドボディにキャスト
	CRigidBody* pRB = DownCast<CRigidBody>(GetCollider());

	// ワールドトランスフォームから位置を取得
	const DirectX::XMFLOAT3& Pos = pRB->GetWorldTransform().Pos;

	//プレイヤーと同じ高さ
	if (Pos.y < 3.0f)
	{
		// 自身の死亡フラグを立てる
		SetDeath();
	}
}

//======================================
//各情報を計算する処理
//======================================
void CEnemy1::Calculation()
{
	std::vector<float>fvSaveDistance;  //各プレイヤーと自身の距離を算出
	std::vector<float>fvAngle;         //各プレイヤーと自身の向きを算出

	auto SelfPos = GetTransform().Pos; //自身の位置 

	//範囲baseでプレイヤーを参照
	for (const auto& pPlayer : m_pPlayer)
	{	
		auto PlayerPos = pPlayer->GetTransform().Pos;                                   //プレイヤーの位置

		fvAngle.emplace_back(atan2f(PlayerPos.x - SelfPos.x, PlayerPos.z - SelfPos.z)); //対角線の角度を算出する（目標ー自機）

		fvSaveDistance.push_back(CheckDistance(PlayerPos, SelfPos));                    //距離を算出
	}

	//-----------*全てのプレイヤーとの計算が終了＝算出したデータを比較し、最も近いプレイヤーを探す*-----------
	auto min_iterator = std::min_element(fvSaveDistance.begin(), fvSaveDistance.end()); //最小値取得
	size_t min_index = std::distance(fvSaveDistance.begin(), min_iterator);             //最小値を持つ番号取得

	//行動処理を呼ぶ
	Action(m_pPlayer[min_index], fvAngle[min_index]); 
}

//======================================
//行動処理
//======================================
void CEnemy1::Action(CPlayer* pPlayer, float fAngle)
{
	auto PlayerPos = pPlayer->GetTransform().Pos;   //プレイヤーの位置
	auto SelfPos = GetTransform().Pos;              //自身の位置 

	const float RADIUS = 5.0f;                      //範囲

	//当たり判定
	if (CheckCollision(PlayerPos, SelfPos, RADIUS))
	{
		ActionInColi();
	}
	else
	{
		ActionOutColi(fAngle);
	}

	//頂点に達した時（ジャンプ時の最高到達点）
	//Top(SelfPos);

	StopAir();
}

//======================================
//行動時範囲内にいる時の処理
//======================================
void CEnemy1::ActionInColi()
{
	//ジャンプしてない
	if (m_bJump)
	{
		m_nRecasttime = 0; //リキャストタイムの初期化
		m_bJump = false;   //jump不可能
		Jump();            //ジャンプ処理を呼ぶ
	}

	//仮にジャンプした後にプレイヤーが範囲内に居座った時
	else
	{
		++m_nRecasttime;

		//リキャストタイムが規定値以上の時（リキャストタイム終了）
		if (m_nRecasttime > MAX_RECASTTIME_IN)
		{
			m_bJump = true;  //jump可能
		}
	}
}

//======================================
//行動時範囲内にいない時の処理
//======================================
void CEnemy1::ActionOutColi(float fAngle)
{
	//リキャストタイムが規定値以上の時（リキャストタイム終了）
	if (m_nRecasttime >= MAX_RECASTTIME)
	{
		m_bJump = true;
		MoveAtPlayer(fAngle, MOVE); //プレイヤーへ移動させる
	}

	//ジャンプした
	else if (!m_bJump)
	{
		++m_nRecasttime;
	}
}

//======================================
//ジャンプ処理
//======================================
void CEnemy1::Jump()
{
	const float Adjsut = 0.15f;

	btVector3 btJumpVec = { 0.0f, JUMPPOWER, 0.0f };

	//リジットボディを取得
	CRigidBody* pRB = DownCast<CRigidBody>(GetCollider());

	// 現在の加速度を参照
	const btVector3& rCurrentVel = pRB->GetLinearVelocity();

	// ジャンプ力：XZ軸：現在の移動方向を逓減して反映
	btJumpVec.setX(rCurrentVel.getX() * Adjsut);
	btJumpVec.setZ(rCurrentVel.getZ() * Adjsut);

	// アクティブ化
	pRB->SetActive();

	// ジャンプ力を反映
	pRB->SetImpulse(btJumpVec);
}

//======================================
//hipDrop処理
//======================================
void CEnemy1::HipDrap()
{
	// ドロップ力
	btVector3 btDropVec = { 0.0f, -JUMPPOWER, 0.0f };

	//リジットボディを取得
	CRigidBody* pRB = DownCast<CRigidBody>(GetCollider());

	// ダイナミックに戻す
	pRB->SetDynamic();

	// アクティブに変更
	pRB->SetActive();

	// ドロップ力を反映
	pRB->SetImpulse(btDropVec);

	//CreateShockWave(Collision::SHAPETYPE::SPHERE, { 2.0f, 2.0f, 2.0f }, 1);
}

//============================================================================
// 衝撃波の作成(未定)
//============================================================================
void CEnemy1::CreateShockWave(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3A& Size, int nDuration)
{
	// 衝撃波の作成
	m_pShockWave = CObject::Create<CShockWave>(OBJ::TYPE::NONE, OBJ::LAYER::DEFAULT);

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
//空中停止処理(未定)
//======================================
void CEnemy1::StopAir()
{
	auto SelfPos = GetTransform().Pos;
	btVector3 Pos;

	//飛んでいる最中
	if (!m_bJump)
	{
		//リジットボディを取得
		CRigidBody* pRB = DownCast<CRigidBody>(GetCollider());

		//自身のY軸の位置が規定値に達した時
		if (SelfPos.y >= TOP_POS_Y&&!m_bTop)
		{
			btVector3   MoveDir = { 0.0f,0.0f,0.0f };                //位置情報設定用

			pRB = DownCast<CRigidBody>(GetCollider());

			Pos = pRB->GetLinearVelocity();

			pRB->SetLinearVelocity(MoveDir);

			m_bTop = true;
		}

		if (m_bTop)
		{
			++m_nTopTimer;

			if (m_nTopTimer > 60)
			{
				m_nTopTimer = 0;
				m_bTop = false;

				pRB->SetLinearVelocity(Pos);
				HipDrap();
			}
		
		}

	}
}

//======================================
//プレイヤーの方へ移動する処理
//======================================
void CEnemy1::MoveAtPlayer(float Angle, float speed)
{
	btVector3   MoveDir = { 0.0f,0.0f,0.0f };                //位置情報設定用

	CRigidBody* pRB = DownCast<CRigidBody>(GetCollider());   //リジットボディを取得

	const btVector3& rCurrentVel = pRB->GetLinearVelocity(); //現在の加速度を参照

	pRB->SetActive();                                        //アクティブ化

	//各位置の設定
	MoveDir.setX(sinf(Angle) * speed);  
	MoveDir.setZ(cosf(Angle) * speed);

	// 移動方向：Y軸：現在の重力速度を維持
	MoveDir.setY(rCurrentVel.getY()); 

	pRB->SetLinearVelocity(MoveDir); //加速度の設定
}

//======================================
//頂点に達した時の処理
//======================================
void CEnemy1::Top(XMFLOAT3 SelfPos)
{
	if (!m_bJump)
	{
		//疑似的に到達点を設定
		if (SelfPos.y >= TOP_POS_Y)
		{
			HipDrap();
		}
	}
}

//======================================
//当たり判定チェック処理
//======================================
bool CEnemy1::CheckCollision(const XMFLOAT3& c1, const XMFLOAT3& c2,float Radius)
{
	//各場所から値を算出
	float lengthX = c1.x - c2.x;
	float lengthY = c1.y - c2.y;
	float lengthZ = c1.z - c2.z;

	float centerDistance = sqrtf(lengthX * lengthX + lengthY * lengthY + lengthZ * lengthZ);

	//半径の和
	float radiusSum =Radius;

	//中心点の距離より半径の和のほうが大きい
	if (centerDistance <= radiusSum)
	{
		return true; //二つの円が当たっている
	}

	return false;	 //二つの円が当たっていない
}

//======================================
//対角線の値を出す処理
//======================================
float CEnemy1::CheckDistance(const XMFLOAT3& c1, const XMFLOAT3& c2)
{
	//各場所から値を算出
	float lengthX = c1.x - c2.x;
	float lengthY = c1.y - c2.y;
	float lengthZ = c1.z - c2.z;
	float centerDistance = sqrtf(lengthX * lengthX + lengthY * lengthY + lengthZ * lengthZ);

	return  centerDistance; //対角線の値を返す
}


//======================================
//描画処理
//======================================
void CEnemy1::Draw()
{
	CPhysicsObject::Draw();
}