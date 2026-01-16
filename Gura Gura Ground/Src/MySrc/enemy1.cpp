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
	const float MOVE = 6.5f;                 //自身の移動値 (今はdebugで似てる速度を目視で設定中)
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
, m_nStart(0), m_bStart(false), m_pwPlayer{}, m_State(ENEMY_STATE::STATE_BASE)
{
	searchPlayer();  //プレイヤーを探す(初めにプレイヤーが生成されてるのが条件)
	searchBar();     //障害物を探す(初めにプレイヤーが生成されてるのが条件)

	//あらかじめパラメータを設定
	m_params.predictionTime = 0.295f +RandomRange(-0.05f, 0.05f); //ある程度の値の大きさを持たせる	// モデルのバインド
	SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Test"));

	m_State = ENEMY_STATE::STATE_BASE;}

//======================================
//デストラクタ
//======================================
CEnemyPlayer::~CEnemyPlayer()
{
	//ポインターの情報を消す
	std::vector<std::weak_ptr<CPlayer>>().swap(m_pwPlayer);
	std::vector<std::weak_ptr<CEnemyPlayer>>().swap(m_pwSelf);
	m_pBar = nullptr;
}

//======================================
//別の自身クラスを探す処理
//======================================
void CEnemyPlayer::searchEnemy(std::shared_ptr<CEnemyPlayer>pSelf)
{
	m_pwSelf.push_back(pSelf);
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

	//各情報を判定し、それに対応した呼び出す
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
	State_Base_Search();
	State_Base_Bar();
}

//======================================
//基本となる状態の処理
//======================================
void CEnemyPlayer::State_Base_Search()
{
	std::vector<TargetInfo> targets;   //まとめて比較する用変数(構造体により、型が違くても比較可能)

	auto SelfPos = GetTransform().Pos; //自身の位置

	//targetsに各情報を入れる
	CollectTargetInfo(m_pwPlayer, targets, SelfPos); //プレイヤーの情報
	CollectTargetInfo(m_pwSelf, targets, SelfPos);   //（別の）自身の情報

	//対象がいなければ終了
	if (targets.empty())
	{
		return; //これがないとerror(情報がない為)
	}

	//最も近いターゲットを取得 
	auto min_it = std::max_element
	(
		targets.begin(), targets.end(),
		[&](const TargetInfo& a, const TargetInfo& b)
		{
			return ScoreTarget(a, SelfPos) < ScoreTarget(b, SelfPos);
		}
	);

	//ここで予測位置を計算する 
	float predictionTime = m_params.predictionTime; 
	DirectX::XMFLOAT3 predictedPos =
	{
		min_it->pos.x + min_it->vel.getX() * predictionTime,
		min_it->pos.y + min_it->vel.getY() * predictionTime,
		min_it->pos.z + min_it->vel.getZ() * predictionTime
	};

	//予測位置への角度を計算
	float predictedAngle = atan2f
	(
		predictedPos.x - SelfPos.x,
		predictedPos.z - SelfPos.z
	);

	//Comparison に渡す angle を差し替える
	Comparison(predictedPos, SelfPos, predictedAngle);
}

//======================================
//比較処理(当たった時の判定や初動動かない処理)
//======================================
void CEnemyPlayer::Comparison(const DirectX::XMFLOAT3 targetPos, const DirectX::XMFLOAT3 SelfPos, float angle)
{
	const float RADIUS = 3.0f;         //当たり半径

	//当たっているかどうか判定
	if (CheckCollision(targetPos, SelfPos, RADIUS))
	{
		if (m_bJump)
		{
			Jump_Base();                             //飛ぶ前の準備段階               
			ChangeState(ENEMY_STATE::STATE_IN_JUMP); //状態をジャンプ中に変更
		}
	}
	else
	{
		//初動が完了
		if (m_bJump && m_bStart)
		{
			MoveAtPlayer(angle, MOVE); //移動
		}
		else if (!m_bStart)
		{
			++m_nStart;

			//初動どれだけ動かないか
			if (m_nStart >= MAX_RECASTTIME_MOVE)
			{
				m_bStart = true;
			}
		}
	}
}

//======================================
//対象の総合判定処理
//======================================
float CEnemyPlayer::ScoreTarget(const TargetInfo& t, const DirectX::XMFLOAT3& selfPos)
{
	float score = 0.0f;

	// -----------------------------
	// 1、距離（最重要）
	// 距離^3 → 距離^2 に変更して安定化
	// -----------------------------
	float distanceScore = 1.0f / (t.distance * t.distance + 0.001f);
	distanceScore = btClamped(distanceScore, 0.0f, 1000.0f);         // 暴走防止
	score += distanceScore * m_params.weightDistance;

	// -----------------------------
	// 2、接近度 * 距離減衰
	// approach を 0～1 に正規化して扱う
	// -----------------------------
	useful::Vec3 toTarget = t.pos - useful::Vec3(selfPos.x, selfPos.y, selfPos.z);
	toTarget = NormalizeFloat3(toTarget); //正規化

	//正規化
	btVector3 velNorm = t.vel;        //normalize関数を使用するために置き換え
	if (velNorm.length2() > 0.0001f)
	{
		velNorm.normalize();
	}
	float approach = btDot(btVector3(toTarget.x, toTarget.y, toTarget.z), velNorm); //「ターゲット方向ベクトル」と「正規化された速度ベクトル」の内積＝接近度」
	float approach01 = (approach + 1.0f) * 0.5f;                                    // [-1,1] → [0,1] に正規化

	// 距離^2 で減衰（距離^3 より安定）
	float approachScore = approach01 / (t.distance * t.distance + 0.001f);
	approachScore = btClamped(approachScore, 0.0f, 1000.0f);
	score += approachScore * m_params.weightApproach;

	return score;
}

//======================================
//正規化
//======================================
XMFLOAT3 CEnemyPlayer::NormalizeFloat3(const DirectX::XMFLOAT3& v)
{
	XMVECTOR vec = XMLoadFloat3(&v);
	vec = XMVector3Normalize(vec);

	XMFLOAT3 out;
	XMStoreFloat3(&out, vec);

	return out;
}

//======================================
//基本となる状態のバーの処理
//======================================
void CEnemyPlayer::State_Base_Bar()
{
	if (m_pBar)
	{
		const float size = 1.0f; //当たり判定の大きさ

		//自身のトランスフォーム情報
		auto SelfTransform = GetTransform();
		XMFLOAT3 SelfSize = { size, size, size };                                                  //「ファクトリーコライダーの値」を参照      
		GameObject self_GO = SetObbInfo(self_GO, SelfTransform.Pos, SelfSize, SelfTransform.Rot);

		//バーのトランスフォーム情報
		CRigidBody* pRB_Bar = DownCast<CRigidBody>(m_pBar->GetCollider());
		auto BarTransform = pRB_Bar->GetWorldTransform();
		XMFLOAT3 BarSize = { size, 15.0f, size };                                                  //「ファクトリーコライダーの値」を参照
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
		auto pPlayer = std::dynamic_pointer_cast<CPlayer>(Obj);
		m_pwPlayer.push_back(pPlayer);
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

//======================================
//当たり判定チェック処理
//======================================
bool CEnemyPlayer::CheckCollision(const XMFLOAT3& c1pos, const XMFLOAT3& c2pos, float Radius)
{
	//対角線を算出
	float centerDistance = CheckDistance(c1pos, c2pos);

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
		if (DownHit(bJump, RecastTme, MaxRecast))
		{
			return true;
		}
	}
	else if (m_bGoDown && Collision::CheckHitToRigidBodyRaw(pRB))
	{
		if (DownHit(bJump, RecastTme, MaxRecast))
		{
			return true;
		}
	}

	// 現在の加速度情報を次フレームへ持ち越し
	m_btOldVel = rCurrentVel;

	return false;
}

//======================================
//落下判定中の処理
//======================================
bool CEnemyPlayer::DownHit(bool& bJump, int& RecastTme, const int MaxRecast)
{
	++RecastTme; //必ず０から始動

	//リキャストタイムが規定値に達した時
	if (RecastTme >= MaxRecast)
	{
		m_bGoDown = false;  //強制的に下降状態を解く（もう地上判定）
		RecastTme = 0;      //リキャストタイムを初期化
		bJump = true;       //ジャンプ可能

		return true;
	}
	//多少強引に一回だけ衝撃波を呼ぶ処理を実行
	else if (RecastTme <= 1)
	{
		CreateShockWave(Collision::SHAPETYPE::BOX, { 7.0f, 1.0f, 7.0f }, 10);
	}

	return false;
}

//======================================
//情報があるかどうか確認処理
//======================================
void CEnemyPlayer::CheckInfo()
{
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
//飛ぶ基本処理
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

//============================================================================
// 衝撃波の作成
//============================================================================
void CEnemyPlayer::CreateShockWave(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3A& Size, int nDuration)
{
	// 衝撃波の作成と、弱参照の設定
	const std::shared_ptr<CShockWave>& spShockWave = CObjectManager::CreateShare<CShockWave>
		(
		  OBJ::TYPE::NONE,
		  OBJ::LAYER::DEFAULT
		);

	// 自身のトランスフォームを出現位置に設定
	spShockWave->SetTransform(GetTransform());

	// ゴーストの作成
	spShockWave->FactoryCollider(Type, Size.x, Size.y, Size.z);

	// 自身を無視対象に設定
	spShockWave->SetIgnore(shared_from_this());

	// 期間の設定
	spShockWave->SetDuration(nDuration);
}


//======================================
//描画処理
//======================================
void CEnemyPlayer::Draw()
{
	CPhysicsModel::Draw();
}