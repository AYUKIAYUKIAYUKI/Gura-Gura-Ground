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
#include "API.object.manager.h"

//================================================
//名前空間（無名）
namespace
{
	btVector3 INIT = { 0.0f, 0.0f, 0.0f };//btVector3用初期化マクロ
}


//======================================
//コンストラクタ
//======================================
CEnemy1::CEnemy1(OBJ::TYPE Type, OBJ::LAYER Layer) :CPhysicsObject(Type, Layer),
m_pPlayer({}), m_pShockWave(nullptr), m_nRecasttime(MAX_RECASTTIME), m_bJump(true)
, m_bGoDown(false), m_btOldVel(INIT)
{
	searchPlayer();  //プレイヤーを探す(初めにプレイヤーが生成されてるのが条件)
}

//======================================
//デストラクタ
//======================================
CEnemy1::~CEnemy1()
{
	std::vector<CPlayer*>().swap(m_pPlayer); //解放処理
	m_pShockWave = nullptr;
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
	pRB->SetAngularFactor(INIT);
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
//プレイヤーの情報を消す ->ゆうきのやつがいいかも
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
//プレイヤーに対する各情報を計算する処理
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
	Action(m_pPlayer[min_index], fvAngle[min_index]);  //一番近いプレイヤー
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
		//リキャストタイムが規定値以上の時（リキャストタイム終了）
		if (m_nRecasttime >= MAX_RECASTTIME)
		{
			MoveAtPlayer(fAngle, MOVE); //プレイヤーへ移動させる
		}
	}

	//頂点に達した時（ジャンプ時の最高到達点）
	if (!m_bJump)
	{
		//疑似的に到達点を設定
		if (SelfPos.y >= TOP_POS_Y)
		{
			HipDrap();
		}

		//ジャンプ後に呼ぶ事でリキャストタイムをインクリメントさせる=空中時間の考慮を無視
		InJump();
	}

	//リキャストタイムが規定値に達した時
	if (m_nRecasttime >= MAX_RECASTTIME)
		m_bJump = true;
}

//======================================
//行動時範囲内にいる時の処理
//======================================
void CEnemy1::ActionInColi()
{
	//ジャンプしてない
	if (m_bJump)
	{
		m_nRecasttime = 0; 
		m_bJump = false;  
		Jump();            //ジャンプ処理を呼ぶ
	}
}

//======================================
//ジャンプ処理
//======================================
void CEnemy1::Jump()
{
	//リジットボディを取得
	CRigidBody* pRB = DownCast<CRigidBody>(GetCollider());

	// 現在の加速度を参照
	const btVector3& rCurrentVel = pRB->GetLinearVelocity();

	//調整値
	const float Adjsut = 0.15f;

	//位置情報設定用
	btVector3 btJumpVec = { 0.0f, JUMPPOWER, 0.0f };

	// ジャンプ力：XZ軸：現在の移動方向を逓減して反映
	btJumpVec.setX(rCurrentVel.getX() * Adjsut);
	btJumpVec.setZ(rCurrentVel.getZ() * Adjsut);

	// アクティブ化
	pRB->SetActive();

	// ジャンプ力を反映
	pRB->SetImpulse(btJumpVec);
}

//======================================
//ジャンプ中の処理
//======================================
void CEnemy1::InJump()
{
	// リジッドボディの取得
	CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// 現在の加速度をコピー
	const btVector3& rCurrentVel = pRB->GetLinearVelocity();

	// 下降判定
	if (!m_bGoDown && rCurrentVel.getY() < 0.0f && m_btOldVel.getY() > 0.0f)
	{
		m_bGoDown = true;
	}

	//下降中に何かに当たった時
	if (m_bGoDown && Collision::GetHitRigidBody(pRB))
	{
		++m_nRecasttime;
	}

	// 現在の加速度情報を次フレームへ持ち越し
	m_btOldVel = rCurrentVel;
}

//======================================
//hipDrop処理
//======================================
void CEnemy1::HipDrap()
{
	//リジットボディを取得
	CRigidBody* pRB = DownCast<CRigidBody>(GetCollider());

	// ダイナミックに戻す
	pRB->SetDynamic();

	// アクティブに変更
	pRB->SetActive();

	// ドロップ力
	btVector3 btDropVec = { 0.0f, -JUMPPOWER, 0.0f };

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
//プレイヤーの方へ移動する処理
//======================================
void CEnemy1::MoveAtPlayer(float Angle, float speed)
{
	CRigidBody* pRB = DownCast<CRigidBody>(GetCollider());   //リジットボディを取得

	const btVector3& rCurrentVel = pRB->GetLinearVelocity(); //現在の加速度を参照

	pRB->SetActive();                                        //アクティブ化

	 //位置情報設定用
	btVector3   MoveDir = INIT;

	//各位置の設定
	MoveDir.setX(sinf(Angle) * speed);  
	MoveDir.setZ(cosf(Angle) * speed);

	// 移動方向：Y軸：現在の重力速度を維持
	MoveDir.setY(rCurrentVel.getY()); 

	pRB->SetLinearVelocity(MoveDir); //加速度の設定
}

//======================================
//当たり判定チェック処理
//======================================
bool CEnemy1::CheckCollision(const XMFLOAT3& c1, const XMFLOAT3& c2,float Radius)
{
	//距離を算出
	auto centerDistance = CheckDistance(c1, c2);

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