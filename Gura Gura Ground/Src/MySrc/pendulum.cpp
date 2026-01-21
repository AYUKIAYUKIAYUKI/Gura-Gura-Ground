
//============================================================================
// 
// 振り子 [pendulum.cpp]
// Author : 大竹熙
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "pendulum.h"
#include "API.sound.manager.h"

// 物理挙動作成のため
#include "API.world.h"
#include "API.collision.h"

// エフェクト
#include "route.h"

//****************************************************
// usingディレクティブ
//****************************************************
using namespace DirectX;
using namespace useful;

//****************************************************
// 無名名前空間の定義
//****************************************************
namespace
{
	// フィールドサイズ
	float g_fFieldSpan = 15.0f;
	float g_fFieldHalf = g_fFieldSpan * 0.5f;

	// 高度
	float g_fAxisY_Spawn = 20.0f; // スポーン高度
	float g_fAxisY_Despawn = 3.0f;  // デスポーン高度

	// 振り子パラメータをまとめる
	namespace PendulumParams
	{
		// ================================
		// --- 振り子の物理パラメータ ---
		// ================================

		// 振り子の角速度（揺れる速さ）
		const float Omega = 0.8f;

		// 振り子の長さ（ステージ幅に対して決まる）
		const float Length = g_fFieldSpan * 1.2f;

		// 最大振れ角（左右にどれだけ振れるか）
		const float MaxAngle = 2.3f;

		// 見た目の Y 方向縮小率（モデル補正用）
		const float ShrinkY = 0.2f;

		// Y 方向のマージン（地面とのめり込み防止）
		const float MarginY = 2.0f;


		// ====================================
		// --- 当たり判定・吹っ飛び関連 ---
		// ====================================

		// 擬似速度計算用のデルタタイム
		const float Dt = 1.0f / 60.0f;

		// 擬似速度の上限（安全装置）
		const float MaxSpeed = 10.0f;

		// ヒット後のクールタイム（連続ヒット防止）
		const int HitCooldown = 10;

		// 振り子の当たり判定の半径
		const float Radius = 1.0f;

		// 横方向の最大ブースト（未使用なら予備）
		const float SideBoostMax = 4.0f;


		// ================================
		// --- 吹っ飛びパワー調整 ---
		// ================================

		// 横方向の吹っ飛び強さ
		const float HorizontalPower = 4.0f;

		// 上方向の吹っ飛び強さ
		const float UpwardBoost = 5.0f;

		// 基本吹っ飛びパワー
		const float BasePower = 15.0f;

		// 振り子の速度に応じて加算されるパワー
		const float AddBySpeed = 10.0f;

		// 最終的な吹っ飛びパワーの上限（安全装置）
		const float MaxFinalPower = 20.0f;


		// ================================
		// --- 見た目関連 ---
		// ================================

		// 振り子モデルの見た目の半径
		const float PendulumRadius = 4.0f;
	}

	// 位置表示
	void Print_Pos(const OBJ::Transform& TF)
	{
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		if (ImGui::TreeNodeEx("Pendulum", ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::Text("Pendulum Pos X: %.2f", TF.Pos.x);
			ImGui::Text("Pendulum Pos Y: %.2f", TF.Pos.y);
			ImGui::Text("Pendulum Pos Z: %.2f", TF.Pos.z);
			ImGui::TreePop();
		}
		ImGui::End();
	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CPendulum::CPendulum(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CObstacle(Type, Layer, Obstacle::OBSTACLE_TYPE::PERIMETER)
	, m_Direction(VEC3_ZERO_INIT)
	, m_Time(0.0f)
{}

//============================================================================
// デストラクタ
//============================================================================
CPendulum::~CPendulum()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CPendulum::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	SetCollider(CRigidBody::CreateRigidBody(
		GetTransform(),
		Collision::SHAPETYPE::SPHERE,
		fWidth, fHeight, fDepth));

	m_pRB = useful::DownCast<CRigidBody>(GetCollider());

	btRigidBody* rb = m_pRB->GetRigidBody();

	rb->setCollisionFlags(rb->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	rb->setActivationState(DISABLE_DEACTIVATION);
	rb->setGravity(btVector3(0, 0, 0));

	Appear();

	/* ！！！ トランスフォームのサイズをコライダーのもので設定 ！！！ */
	OBJ::Transform TF = {};
	TF.Size = { fWidth, fHeight, fDepth };
	SetTransform(TF);

	/* ！！！ 警告表示の作成 ！！！*/
	CRoute* pRoute = CObjectManager::CreateRaw<CRoute>();
	std::shared_ptr<CObstacle> spObstacle = std::dynamic_pointer_cast<CObstacle>(shared_from_this());
	pRoute->SetTrackTarget(spObstacle);
}

//============================================================================
// 更新処理
//============================================================================
void CPendulum::Update()
{
	const float dt = 1.0f / 60.0f; 
	m_Time += dt;

	// 挙動
	Action();

	// 1往復周期 = 2π / ω（端→端）
	const float period = 2.0f * DirectX::XM_PI / PendulumParams::Omega;

	// 現在のワールドトランスフォームを取得
	OBJ::Transform TF = {};
	m_pRB->GetWorldTransform(TF);

	if (m_Time >= period)
	{
		// 自身の死亡フラグを立てる
		SetDeath();
	}

	// 戻る
	//Loop();

	CheckHitPlayer();

	// 物理オブジェクト用の更新：WVP行列用定数バッファの更新
	CPhysicsObject::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CPendulum::Draw()
{
	// 物理オブジェクト用の描画：モデルの描画
	CPhysicsObject::Draw();
}

//============================================================================
// 出現
//============================================================================
void CPendulum::Appear()
{
	OBJ::Transform TF = {};

	// エディターで設定している値を設定
	const auto& param = m_ObstacleEditer.m_ParamSets[GetParamSetIndex()].subParams[GetSubParamIndex()];
	m_OriginPos = { param.ObstacleSpawnX, g_fAxisY_Spawn, param.ObstacleSpawnZ };

	// 移動速度スケール作成
	const float fSpeed = 3.0f;

	// エディターで設定している値を適応
	TF.Pos = { param.ObstacleSpawnX, g_fAxisY_Spawn, param.ObstacleSpawnZ };
	SetDirection({ param.ObstacleSpeedX, g_fAxisY_Spawn, param.ObstacleSpeedZ });

	m_Phase = -DirectX::XM_PI * 0.5f;

	// ワールドトランスフォームに反映
	m_pRB->SetWorldTransform(TF);

	m_Time = 0.0f;
	m_hasPrevPos = false;
}

//============================================================================
// 挙動
//============================================================================
void CPendulum::Action()
{
	if (!m_pRB) return;

	const float theta = PendulumParams::MaxAngle * sinf(PendulumParams::Omega * m_Time + m_Phase);

	const float threshold = 0.8f; // ← ここを大きくするともっと早く鳴る
	if (fabs(theta) < threshold)
	{
		if (!m_hasPlayedNearCenter)
		{
			CSoundManger::RefInstance().Play("Pendulum", false, -0.5f, 1.0f);
			m_hasPlayedNearCenter = true;
		}
	}
	else
	{
		m_hasPlayedNearCenter = false;
	}

	const float groundY = g_fAxisY_Despawn;
	const float r = PendulumParams::PendulumRadius;
	const float safety = PendulumParams::MarginY;

	const float minCenterY = groundY + r + safety;
	const float Ls = PendulumParams::Length * PendulumParams::ShrinkY;
	const float pivotY = minCenterY + Ls;

	const float swing = PendulumParams::Length * sinf(theta);
	const float swingY = pivotY - Ls * cosf(theta);

	OBJ::Transform TF{};

	// エディターで設定した値が運動基点として加算し、中心座標となるようにする
	if (m_Direction.z != 0.0f)
	{
		TF.Pos.x = m_OriginPos.x;
		TF.Pos.y = swingY;
		TF.Pos.z = m_OriginPos.z + swing;
	}
	else
	{
		TF.Pos.x = m_OriginPos.x + swing;
		TF.Pos.y = swingY;
		TF.Pos.z = m_OriginPos.z;
	}
	m_pRB->SetWorldTransform(TF);
}

//============================================================================
// 戻る
//============================================================================
void CPendulum::Loop()
{
	// 1往復周期 = 2π / ω（端→端）
	const float period = 2.0f * DirectX::XM_PI / PendulumParams::Omega;

	if (!m_pRB) return;

	// 現在のワールドトランスフォームを取得
	OBJ::Transform TF = {};
	m_pRB->GetWorldTransform(TF);

	if (m_Time >= period) 
	{
		m_Time = 0.0f;   // 時間リセット

		SetDeath();

		// 塵：拡散発生
		//CDust::GenerateSpread(TF.Pos, 10);

		// ★ 振り子が戻ったのでヒット履歴をリセット
		m_HitPlayers.clear();
	}

	/* 位置を出力*/
	Print_Pos(TF);
}

//============================================================================
// プレイヤーとの当たり判定
//============================================================================
void CPendulum::CheckHitPlayer()
{
	if (!m_pRB) return;

	CRigidBody* const pPendulumRB = m_pRB;

	OBJ::Transform tfPend{};
	pPendulumRB->GetWorldTransform(tfPend);

	// ★ 振り子の擬似速度（3D）
	btVector3 pseudoVel(0, 0, 0);
	if (m_hasPrevPos)
	{
		pseudoVel = btVector3(
			tfPend.Pos.x - m_prevPos.x,
			tfPend.Pos.y - m_prevPos.y,
			tfPend.Pos.z - m_prevPos.z
		) / PendulumParams::Dt;
	}

	float speed = pseudoVel.length();
	if (speed > PendulumParams::MaxSpeed)
		speed = PendulumParams::MaxSpeed;

	// ★ 折り返し検出用：前フレームの速度を static で保持
	static float prevVelX = 0.0f;

	// 今の速度
	float velX = pseudoVel.x();

	// ★ 速度の符号が反転したら折り返し（往復の切り替わり）
	if (prevVelX * velX < 0.0f)
	{
		m_HitPlayers.clear(); // ← これで戻りでも当たれる
	}

	prevVelX = velX;

	// ★ クールタイム（連続ヒット防止）
	if (m_HitCooldown > 0)
	{
		--m_HitCooldown;
		m_prevPos = tfPend.Pos;
		m_hasPrevPos = true;
		return;
	}

	// プレイヤー全員チェック
	const auto& rPlayerList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);
	for (const auto& rIt : rPlayerList)
	{
		CPlayer* pPlayer = dynamic_cast<CPlayer*>(rIt.get());
		if (!pPlayer) continue;

		CRigidBody* const pPlayerRB = useful::DownCast<CRigidBody>(pPlayer->GetCollider());
		if (!pPlayerRB) continue;

		// ★ この振り子はこのプレイヤーにもう当たったか？
		if (m_HitPlayers.count(pPlayer) > 0)
		{
			continue; // 再ヒット禁止
		}

		// ★ 接触判定
		Collision::MyContactCallbackRigidBodyAndRigidBody callback(pPendulumRB, pPlayerRB);
		CWorld::RefInstance().RefDynamicsWorldConst()->contactPairTest(
			pPendulumRB->GetRigidBody(),
			pPlayerRB->GetRigidBody(),
			callback
		);
		if (!callback.m_bHit) continue;

		// ★ 初回ヒットとして記録（これが最重要）
		m_HitPlayers.insert(pPlayer);

		// ★★★ 吹っ飛び方向 = 振り子の進行方向 ★★★
		btVector3 moveDir = pseudoVel;
		if (moveDir.length() < 1e-6f)
			moveDir = btVector3(1, 0, 0);

		moveDir = moveDir.normalized();

		// ★ 横方向 3倍 + 上方向 1.0倍（最適バランス）
		btVector3 dir = moveDir * 3.0f + btVector3(0, 1.0f, 0);
		dir = dir.normalized();

		// ★ プレイヤーの抵抗（後ろ飛び防止）
		btVector3 playerVel = pPlayerRB->GetLinearVelocity();
		float resist = -playerVel.dot(moveDir);
		if (resist < 0.0f) resist = 0.0f;

		const float resistFactor = 0.2f;

		// ★ パワー計算（弱め版）
		float t = speed / PendulumParams::MaxSpeed;

		float power = PendulumParams::BasePower
			+ PendulumParams::AddBySpeed * t
			+ resist * resistFactor;

		// ★ 全体的な吹っ飛び強度のスケール調整
		power *= 4.0f;

		// ★ 吹っ飛び強度の最大値を制限（暴走防止）
		power = std::clamp(power, 0.0f, PendulumParams::MaxFinalPower * 5.0f);

		// ★ 少し浮かせる（床抜け防止）
		{
			OBJ::Transform tf = {};
			pPlayerRB->GetWorldTransform(tf);
			tf.Pos.y += 0.1f;
			pPlayerRB->SetWorldTransform(tf);
		}

		// ★ Impulse（メイン吹っ飛び）
		pPlayerRB->SetActive();
		pPlayerRB->SetImpulse(dir * power);

		// ★ 速度制限（強化）
		{
			btVector3 v = pPlayerRB->GetLinearVelocity();
			if (v.getY() < 0.0f) v.setY(0.0f);

			const float MaxPlayerSpeed = 70.0f;
			float len = v.length();
			if (len > MaxPlayerSpeed)
				v *= (MaxPlayerSpeed / len);

			pPlayerRB->SetLinearVelocity(v);
		}

		// ★ クールタイム
		m_HitCooldown = 25;
		break;
	}

	m_prevPos = tfPend.Pos;
	m_hasPrevPos = true;
}
