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
#include "API.sound.manager.h"

std::vector<float> CEnemyPlayer::s_vSurvivalTimes = {};

//================================================
//名前空間（無名）
namespace
{
	//===================================================
	//マクロ定義
	btVector3 INIT = { 0.0f, 0.0f, 0.0f };   //btVector3用初期化マクロ

	const int MAX_RECASTTIME = 60;           //プレイヤーのリキャストタイムの最大値
	const int MAX_RECASTTIME_MOVE = 240;     //ゲーム開始時の移動までの時間
	const int MAX_RECASTTIME_IN_BAR = 60;    //バーのリキャストタイムの最大値

	const float MOVE_SPEED = 9.0f;
	const float PREDICTION_TIME_DEF = 0.15f;  //デフォルトで足し合わせる未来視の時間
	const float PREDICTION_TIME = 0.15f;      //乱数で出す未来視の最小大数
	const float ShockWaveSize = 6.0f;         //衝撃波（ｘ。ｚ）の大きさ

	//プレイヤーと同じ数値にする&&プレイヤーから直接同期->処理も変更しないと多分無理
	const float JUMPPOWER = 13.5f;           //自身のジャンプ力
	const float DROPPOWER = JUMPPOWER * 1.5f;  //自身のドロップ速度
}

//================================================
//名前空間（使用）
using namespace DirectX;
using namespace useful;

#include "API.gltf.manager.h"
#include <shadow.h>
#include <field.h>
#include <windfield.h>
#include <effect.manager.h>

//======================================
//コンストラクタ
//======================================
CEnemyPlayer::CEnemyPlayer(OBJ::TYPE Type, OBJ::LAYER Layer) :CPhysicsModel(Type, Layer)
, m_nRecasttime(0), m_bJump(true), m_pBar(nullptr)
, m_pShockWave(nullptr), m_bGoDown(false), m_btOldVel(INIT)
, m_nStart(0), m_bStart(false), m_pwPlayer{}, m_State(ENEMY_STATE::STATE_BASE)
{
	searchPlayer();  //プレイヤーを探す(初めにプレイヤーが生成されてるのが条件)
	searchBar();     //障害物を探す(初めにプレイヤーが生成されてるのが条件)

	//あらかじめパラメータを設定
	m_params.predictionTime = PREDICTION_TIME_DEF + RandomRange(0.0f, PREDICTION_TIME); //ある程度の値の大きさを持たせる	
	m_params.noiseangle = RandomSplit(0.15f, 0.25f);                                    //角度の調整値
	SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Test"));             // モデルのバインド

	// シェアポインタのオブジェクトリストの参照
	const std::list<std::shared_ptr<CObject>>& rFieldList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::FIELD);

	// フィールドの弱参照を設定
	m_wpField = std::dynamic_pointer_cast<CField>(rFieldList.front());
	m_wpWindoField = std::dynamic_pointer_cast<CWindField>(rFieldList.front());
}

//======================================
//デストラクタ
//======================================
CEnemyPlayer::~CEnemyPlayer()
{

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
	// 自身のリジッドボディの作成
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::BOX, fWidth, fHeight, fDepth));

	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(GetCollider());

	// 重力の設定
	pRigidBody->SetGravity({ 0.0f, -25.0f, 0.0f });

	// 摩擦力を設定
	pRigidBody->SetFriction(1.0f);

	// 減衰力を設定
	pRigidBody->SetDamping(0.25f, 0.0f);

	// Y軸以外の回転をロック
	pRigidBody->SetAngularFactor({ 0.0f, 0.0f, 0.0f });

	// 影の作成
	CShadow* pShadow = CObjectManager::CreateRaw<CShadow>(
		OBJ::TYPE::NONE,
		OBJ::LAYER::DEFAULT);

	// 影の追従対象として自身を設定
	pShadow->SetTrackTarget(shared_from_this());
}


//======================================
//更新処理
//======================================
void CEnemyPlayer::Update()
{
	//生存時間の計測
	if (!GetDeath()) 
	{
		if (m_wIdxCPU < s_vSurvivalTimes.size()) {
			s_vSurvivalTimes[m_wIdxCPU] += 1.0f / 60.0f; // 60FPS想定
		}
	}

	//情報があるか確認
	CheckInfo();

	//各情報を判定し、それに対応した呼び出す
	switch (m_State)
	{
	case ENEMY_STATE::STATE_BASE:    State_Base();    break;
	case ENEMY_STATE::STATE_IN_JUMP: State_In_Jump(); break;
	case ENEMY_STATE::STATE_BAR:     State_Bar();     break;
	}
	// 衝撃波の作成と、弱参照の設定


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

	const auto SelfPos = GetTransform().Pos; //自身の位置

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

	float TragetAngle = predictedAngle + m_params.noiseangle;

	//Comparison に渡す angle を差し替える
	Comparison(predictedPos, SelfPos, TragetAngle);
}

//======================================
//比較処理(当たった時の判定や初動動かない処理)
//======================================
void CEnemyPlayer::Comparison(const DirectX::XMFLOAT3& targetPos, const DirectX::XMFLOAT3& SelfPos, float angle)
{
	// 初動がまだ終わっていない
	if (!m_bStart)
	{
		if (++m_nStart >= MAX_RECASTTIME_MOVE)
			m_bStart = true;

		return; // 初動中はここで終了
	}

	//飛べる
	if (m_bJump)
	{
		//定期的に未来視をリセットするこ事で単調さを消す
		if (m_params.jumpcount > 2)
		{
			// 未来視リセット
			m_params.predictionTime = PREDICTION_TIME_DEF + RandomRange(0.0f, PREDICTION_TIME);
			m_params.jumpcount = 0;
		}


		//範囲内
		if (CheckCollision(targetPos, SelfPos, ShockWaveSize * 0.5f))
		{
if (SelfPos.y > targetPos.y)
			{
				MoveAtPlayer(angle + 1.57f, MOVE_SPEED); //移動(目標向きを外側に明示的に修正)
				return;                                   //ジャンプループを防ぐ
			}			++m_params.jumpcount;
			Jump_Base();
			ChangeState(ENEMY_STATE::STATE_IN_JUMP);

			//サウンド再生
			CSoundManger::RefInstance().Play("Jump", false, 0.0f, 1.0f);
		}
		else
		{
			MoveAtPlayer(angle, MOVE_SPEED); //移動
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
		const auto SelfTransform = GetTransform();
		XMFLOAT3 SelfSize = { size, size, size };                                                  //「ファクトリーコライダーの値」を参照      
		GameObject self_GO = SetObbInfo(self_GO, SelfTransform.Pos, SelfSize, SelfTransform.Rot);

		//バーのトランスフォーム情報
		CRigidBody* pRB_Bar = DownCast<CRigidBody>(m_pBar->GetCollider());
		const auto BarTransform = pRB_Bar->GetWorldTransform();
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
	const auto  playerlist = CObjectManager::RefInstance().RefInstance().RefListShare(OBJ::TYPE::PLAYER);

	//範囲baseでプレイヤー情報の基盤を取得
	for (const auto Obj : playerlist)
	{
		//キャストしてプレイヤーの情報を入れる
		auto pPlayer = std::dynamic_pointer_cast<CPlayer>(Obj);
		m_pwPlayer.push_back(pPlayer);
	}

}

//======================================
//プレイヤーの方へ移動する処理(角度、移動速度)
//======================================
void CEnemyPlayer::MoveAtPlayer(const float Angle, const float speed)
{
	// プレイヤーのリジッドボディの取得
	CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(GetCollider());

	// 現在の加速度をコピー
	const btVector3& rCurrentVel = pRigidBody->GetLinearVelocity();

	// 数値を先行して取得
	float fDirectionValue = Angle;

	// 移動速度スケールの作成
	//const float fSpeed = fSpeedArg;
	btVector3   MoveDir = { 0.0f, 0.0f, 0.0f };

// ★★★ プレイヤーが保持しているフィールド参照から、現在のフィールドタイプを判定する ★★★
	CField* pField =GetCurrentField();
	bool bIce = (pField && pField->GetFieldType() == FIELD_TYPE::ICE);
	// 移動方向：XZ軸：方向に沿って単位ベクトルに速度係数を掛けたものを設定
	MoveDir.setX(sinf(fDirectionValue));
	MoveDir.setZ(cosf(fDirectionValue));

	// ★★★ フィールドタイプに応じて移動処理を切り替える（ICE → #if1、NORMAL → #else） ★★★
	if (bIce)
	{
		// 力を加える 
		if (pRigidBody->GetActive())
		{
			pRigidBody->SetForce(MoveDir * speed);
		}
		else
		{
			pRigidBody->SetActive();
			pRigidBody->SetLinearVelocity(MoveDir);
		}
	}
	else
	{
		MoveDir.setY(rCurrentVel.getY());

		// 目標の加速度作成
		const btVector3& TargetVel = MoveDir * speed;

		/* ああ…btVector3をXMFLOAT3に変換 */
		DirectX::XMFLOAT3 CurrentVel_XMFLOAT = { rCurrentVel.getX(), 0.0f, rCurrentVel.getZ() };
		DirectX::XMFLOAT3 TargeVel_XMFLOAT = { TargetVel.getX(),   0.0f, TargetVel.getZ() };

		/* ああ…要素ずつ指数減衰 */
		float fCoef = 0.25f;

		////何かしらのデバフが有効なら慣性に倍率を掛ける
		//if (rStateMachine.m_rPalyer.GetFallTetraBehavior() != nullptr) {
		//	float Inertia = rStateMachine.m_rPalyer.GetFallTetraBehavior()->GetInertiaValue();
		//	fCoef *= Inertia;
		//}
		useful::ExponentialDecay(CurrentVel_XMFLOAT.x, TargeVel_XMFLOAT.x, fCoef);
		useful::ExponentialDecay(CurrentVel_XMFLOAT.z, TargeVel_XMFLOAT.z, fCoef);

		/* ああ…XMFLOAT3の減衰結果をbtVector3に変換 */
		btVector3 ResultVel = { CurrentVel_XMFLOAT.x, rCurrentVel.getY(), CurrentVel_XMFLOAT.z };

		/* 接地しているかどうか (便宜的にシェアポインタのリジッドボディに接触しているか) に応じて速度の加え方を変更 */
		pRigidBody->SetActive();
		if (Collision::CheckHitToRigidBodyShare(pRigidBody))
		{
			pRigidBody->SetLinearVelocity(ResultVel);
		}
		else
		{
			pRigidBody->SetForce((ResultVel - rCurrentVel) * 10.0f);
		}
	}
}

//======================================
//当たり判定チェック処理
//======================================
bool CEnemyPlayer::CheckCollision(const XMFLOAT3& c1pos, const XMFLOAT3& c2pos, const float Radius)
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
	const auto Obstaclelist = CObjectManager::RefInstance().RefInstance().RefListShare(OBJ::TYPE::OBSTACLE);

	//範囲baseで探す
	for (const auto& Obj : Obstaclelist)
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
		CreateShockWave(Collision::SHAPETYPE::CYLINDER, { ShockWaveSize, 1.0f, ShockWaveSize }, 10);

		//ドロップサウンド再生
		CSoundManger::RefInstance().Play("Drop", false, 0.0f, 1.0f);
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

	// トランスフォームから高さを取得
	float fSelfPosY = GetTransform().Pos.y;

	// フィールドの高さを保有
	float fFieldPosY = 0.0f;

	// フィールドの高さを取得
	if (std::shared_ptr<CField> spField = m_wpField.lock())
	{
		fFieldPosY = spField->GetTransform().Pos.y;
	}

	// 風フィールドの高さを取得
	else if (std::shared_ptr<CWindField> spField1 = m_wpWindoField.lock())
	{
		fFieldPosY = spField1->GetTransform().Pos.y;
	}

	// Y座標がフィールドの高さを下回ったら
	if (fSelfPosY < fFieldPosY)
	{
		//ドロップサウンド再生
		CSoundManger::RefInstance().Play("Falling", false, 0.0f, 1.2f);

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
useful::Vec3 EffectVec3 = { GetTransform().Pos.x,6.25f,GetTransform().Pos.z };
	CEffect::Create(CEffectManager::EFFECT_TAG::TAG_HIPDROP, EffectVec3, nullptr, 1.6f);	// 衝撃波の作成と、弱参照の設定
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

//============================================================================
//乱数
//============================================================================
float CEnemyPlayer::RandomRange(float min, float max)
{
	static std::mt19937 mt{ std::random_device{}() };
	std::uniform_real_distribution<float> dist(min, max);
	return dist(mt);
}

//============================================================================
//min～maxの間の数値を乱数で渡し１/２で+-が変わる
//============================================================================
float CEnemyPlayer::RandomSplit(float min, float max)
{
	static std::mt19937 mt(std::random_device{}());

	std::uniform_real_distribution<float> dist(min, max);

	// coinに代入した数値分　trueが出やすくなる（max 1.0）例coin(0.7)-> true:false=0.7:0.3の確率
	std::bernoulli_distribution coin(1.0);

	float v = dist(mt);
	v = coin(mt) ? v : -v;

	// 小数第2位に丸める
	v = std::round(v * 100.0f) / 100.0f;

	return v;
}

//======================================
//描画処理
//======================================
void CEnemyPlayer::Draw()
{
	CPhysicsModel::Draw();
}