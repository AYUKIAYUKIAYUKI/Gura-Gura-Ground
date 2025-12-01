//============================================================================
// 
// プレイヤー [player.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "player.h"
#include "API.rigidbody.h"
#include "API.input.manager.h"
#include "API.world.h"

// 当たり判定用
#include "API.object.manager.h"
#include "API.collision.h"
#include "field.h"
#include "shockwave.h"

// 塵発生用
#include "dust.h"

//****************************************************
// 無名名前空間の定義
//****************************************************
namespace
{
	// 条件制御
	int g_nStopCounter = 10;

	// 運動神経
	float g_fXZAxis_Speed = 8.0f;
	float g_fYAxis_Jump   = 8.0f;

	// 着地の近似値
	float fLandEpsilon = 0.003f;

	// デバッグ用
	void DebugGui()
	{
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		ImGui::DragFloat("Land Epsilon", &fLandEpsilon, 0.001f, 0.001f, 0.1f);
		ImGui::DragInt("Stop Counter", &g_nStopCounter, 1);
		ImGui::Separator();
		ImGui::End();
	}
}

//****************************************************
// ステート構造体の定義
//****************************************************
struct State
{
	//****************************************************
	// special funciton
	//****************************************************
	virtual ~State() = default;

	//****************************************************
	// funciton
	//****************************************************

	// 更新処理
	virtual void Execute(CPlayer::StateMachine& rStateMachine) = 0;
};

//****************************************************
// ステートマシン構造体の定義
//****************************************************
struct CPlayer::StateMachine
{
	//****************************************************
	// special funciton
	//****************************************************
	StateMachine(CPlayer& rPlayer); // デフォルトコンストラクタ

	//****************************************************
	// funciton
	//****************************************************
	void ExecuteState();                                   // 状態実行
	void ChangeState(std::unique_ptr<State>&& upNewState); // 状態変更

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<State> m_upState; // 状態
	CPlayer&               m_rPalyer; // プレイヤーの参照
};

//****************************************************
// デフォルトステート構造体の定義
//****************************************************
struct StateDefault : public State
{
	//****************************************************
	// funciton
	//****************************************************

	// 更新処理
	void Execute(CPlayer::StateMachine& rStateMachine) override;
};

//****************************************************
// ジャンプステート構造体の定義
//****************************************************
struct StateJump : public State
{
	//****************************************************
	// funciton
	//****************************************************

	// 更新処理
	void Execute(CPlayer::StateMachine& rStateMachine) override;

	//****************************************************
	// Data
	//****************************************************
	bool      m_bGoDown  = false;                // 下降判定
	btVector3 m_btOldVel = { 0.0f, 0.0f, 0.0f }; // 過去の加速度
};

//****************************************************
// ドロップステート構造体の定義
//****************************************************
struct StateDrop : public State
{
	//****************************************************
	// funciton
	//****************************************************

	// 更新処理
	void Execute(CPlayer::StateMachine& rStateMachine) override;

	//****************************************************
	// Data
	//****************************************************
	int m_nStopCounter = 0; // 停止期間カウンター
};

//============================================================================
// デフォルトコンストラクタ：ステートマシン
//============================================================================
CPlayer::StateMachine::StateMachine(CPlayer& rPlayer)
	: m_upState(std::make_unique<StateDefault>())
	, m_rPalyer(rPlayer)
{}

//============================================================================
// 状態実行：ステートマシン
//============================================================================
void CPlayer::StateMachine::ExecuteState()
{
	if (m_upState)
	{
		m_upState->Execute(*this);
	}
}

//============================================================================
// 状態変更：ステートマシン
//============================================================================
void CPlayer::StateMachine::ChangeState(std::unique_ptr<State>&& upNewState)
{
	if (!upNewState)
	{
	 	return;
	}

	m_upState = std::move(upNewState);
}

//============================================================================
// 状態実行：デフォルトステート
//============================================================================
void StateDefault::Execute(CPlayer::StateMachine& rStateMachine)
{
	// 制御不能期間が設定されている間処理しない
	if (rStateMachine.m_rPalyer.GetLostControlDuration() > 0)
	{
		return;
	}

	// リジッドボディの取得
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());

	// 現在の加速度を参照
	const btVector3& rCurrentVel = pRB->GetLinearVelocity();

	// 割り当てが該当するゲームパッドの方向入力を取得
	const std::optional<float>& opDir = CInputManager::RefInstance().ConvertInputToMoveDirection(rStateMachine.m_rPalyer.GetIdxPlayer());

	// 方向入力があるなら
	if (opDir)
	{
		// 移動速度スケール
		const float fSpeed  = g_fXZAxis_Speed;
		btVector3   MoveDir = { 0.0f, 0.0f, 0.0f };

		// アクティブ化
		pRB->SetActive();

		// 移動方向：XZ軸：方向に沿って単位ベクトルに速度係数を掛けたものを設定
		MoveDir.setX(sinf(opDir.value()) * fSpeed);
		MoveDir.setZ(cosf(opDir.value()) * fSpeed);

		// 移動方向：Y軸：現在の重力速度を維持
		MoveDir.setY(rCurrentVel.getY() + 0.02f); // ちょい浮遊

		// 新しい加速度をセット
		pRB->SetLinearVelocity(MoveDir);

		// 塵の進行更新
		rStateMachine.m_rPalyer.UpdateDustStep({ -MoveDir.getX(), -MoveDir.getY(), -MoveDir.getZ() });
	}

	// ジャンプ
	if (CInputManager::RefInstance().GetTrackerGamePad(rStateMachine.m_rPalyer.GetIdxPlayer()).a == DirectX::GamePad::ButtonStateTracker::PRESSED)
	{
		// ジャンプ力
		float     fAntiAir  = 0.25f;
		btVector3 btJumpVec = { 0.0f, g_fYAxis_Jump, 0.0f };

		// アクティブ化
		pRB->SetActive();

		// ジャンプ力：XZ軸：現在の移動方向を逓減して反映
		btJumpVec.setX(rCurrentVel.getX() * fAntiAir);
		btJumpVec.setZ(rCurrentVel.getZ() * fAntiAir);

		// ジャンプ力を反映
		pRB->SetImpulse(btJumpVec);

		// ジャンプ状態に変更
		rStateMachine.ChangeState(std::make_unique<StateJump>());
	}
}

//============================================================================
// 状態実行：ジャンプステート
//============================================================================
void StateJump::Execute(CPlayer::StateMachine& rStateMachine)
{
	// 制御不能期間が設定されている間処理しない
	if (rStateMachine.m_rPalyer.GetLostControlDuration() > 0)
	{
		return;
	}

	// リジッドボディの取得
	CRigidBody* const pRB = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());

	// 現在の加速度をコピー
	const btVector3& rCurrentVel = pRB->GetLinearVelocity();

	// 下降判定
	if (!m_bGoDown && rCurrentVel.getY() < 0.0f && m_btOldVel.getY() > 0.0f)
	{
		m_bGoDown = true;
	}

	// 割り当てが該当するゲームパッドの方向入力を取得
	const std::optional<float>& opDir = CInputManager::RefInstance().ConvertInputToMoveDirection(rStateMachine.m_rPalyer.GetIdxPlayer());

	// 方向入力があるなら
	if (opDir)
	{
		// 移動速度スケール
		const float fAirSpeed = g_fXZAxis_Speed;
		btVector3   MoveDir   = { 0.0f, 0.0f, 0.0f };

		// アクティブ化
		pRB->SetActive();

		// 移動方向：XZ軸：方向に沿って単位ベクトルに速度係数を掛けたものを設定
		MoveDir.setX(sinf(opDir.value()) * fAirSpeed);
		MoveDir.setZ(cosf(opDir.value()) * fAirSpeed);

		// 移動方向：Y軸：現在の重力速度を維持
		MoveDir.setY(rCurrentVel.getY());

		// 新しい加速度をセット
		pRB->SetLinearVelocity(MoveDir);
	}

	// 状態変更
	if (m_bGoDown && CInputManager::RefInstance().GetTrackerGamePad(rStateMachine.m_rPalyer.GetIdxPlayer()).a == DirectX::GamePad::ButtonStateTracker::PRESSED)
	{
		// キネマティック化
		pRB->SetKinematic();

		// 下降判定後、ジャンプ状態中に追加入力でドロップ状態に変更
		rStateMachine.ChangeState(std::make_unique<StateDrop>());
	}
	else if (m_bGoDown && Collision::GetHitRigidBody(pRB))
	{
		// 下降判定中に何かに剛体に接触していたら通常状態に変更
		rStateMachine.ChangeState(std::make_unique<StateDefault>());
	}

	// 現在の加速度情報を次フレームへ持ち越し
	m_btOldVel = rCurrentVel;

	/* 現在のY軸の加速度を取得 */
	useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
	ImGui::Text("VelY: %.2f", rCurrentVel.getY());
	ImGui::End();
}

//============================================================================
// 状態実行：ドロップステート
//============================================================================
void StateDrop::Execute(CPlayer::StateMachine& rStateMachine)
{
	// 停止期間カウンターは常時動作
	++m_nStopCounter;

	// リジッドボディの取得
	CRigidBody* const pRB = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());

	// 現在の加速度を参照
	const btVector3& rCurrentVel = pRB->GetLinearVelocity();

	// 現在のワールドトランスフォームをコピー
	OBJ::Transform TF = pRB->GetWorldTransform();

	if (m_nStopCounter < g_nStopCounter)
	{
		// 位置を少しずらす
		TF.Pos =
		{
		   TF.Pos.x + useful::GetRandomValue<float>() * 0.0005f,
		   TF.Pos.y + useful::GetRandomValue<float>() * 0.0005f,
		   TF.Pos.z + useful::GetRandomValue<float>() * 0.0005f
		};

		// トランスフォームをモーションステートとリジッドボディに反映する
		pRB->SetWorldTransform(TF);
	}
	else
	{
		// ドロップ力
		btVector3 btDropVec = { 0.0f, -g_fYAxis_Jump, 0.0f };

		// ダイナミックに戻す
		pRB->SetDynamic();

		// アクティブに変更
		pRB->SetActive();

		// ドロップ力を反映
		pRB->SetImpulse(btDropVec * 3.0f);

		// 衝撃波の作成
		rStateMachine.m_rPalyer.CreateShockWave(Collision::SHAPETYPE::SPHERE, { 2.0f, 2.0f, 2.0f }, 1);
	}

	// 何かリジッドボディとの衝突が確認出来たら
	if (Collision::GetHitRigidBody(pRB))
	{
		// 衝撃波の作成
		rStateMachine.m_rPalyer.CreateShockWave(Collision::SHAPETYPE::CYLINDER, { 7.0f, 1.0f, 7.0f }, 10);

		// 通常状態に変更
		rStateMachine.ChangeState(std::make_unique<StateDefault>());
	}

	/* 現在のY軸の加速度を取得 */
	useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
	ImGui::Text("VelY: %.2f", rCurrentVel.getY());
	ImGui::End();
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CPlayer::CPlayer(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CPhysicsObject(Type, Layer)
	, m_upStateMachine(std::make_unique<StateMachine>(*this))
	, m_pShockWave(nullptr)
	, m_wIdxPlayer(0)
	, m_nLostControlDuration(0)
	, m_nStepCounter(0)
{}

//============================================================================
// デストラクタ
//============================================================================
CPlayer::~CPlayer()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CPlayer::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
	// プレイヤー用のリジッドボディの作成
	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::BOX, fWidth, fHeight, fDepth));

	// コライダーをリジッドボディにキャスト
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(GetCollider());

	// Y軸以外の回転をロック
	pRB->SetAngularFactor({ 0.0f, 0.0f, 0.0f });
}

//============================================================================
// 衝撃波の作成
//============================================================================
void CPlayer::CreateShockWave(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3A& Size, int nDuration)
{
	// 衝撃波の作成
	m_pShockWave = CObject::Create<CShockWave>(OBJ::TYPE::NONE, OBJ::LAYER::DEFAULT);

	// プレイヤーの登録
	m_pShockWave->SetPlayer(this);

	// プレイヤーのトランスフォームを出現位置に設定
	m_pShockWave->SetTransform(GetTransform());

	// ゴーストの作成
	m_pShockWave->FactoryCollider(Type, Size.x, Size.y, Size.z);

	// 衝撃波の作成
	m_pShockWave->SetDuration(nDuration);
}

//============================================================================
// 衝撃波の削除
//============================================================================
void CPlayer::DeleteShockWave()
{
	m_pShockWave = nullptr;
}

//============================================================================
// 更新処理
//============================================================================
void CPlayer::Update()
{
	/* デバッグ用 */
	//DebugGui();

	// 制御不能期間は常にデクリメント
	--m_nLostControlDuration;

	// 状態実行
	if (m_upStateMachine)
	{
		m_upStateMachine->ExecuteState();
	}

	// WVP行列用定数バッファの更新
	CPhysicsObject::Update();

	/*------------------------------------*/

	// コライダーをリジッドボディにキャスト
	CRigidBody* const pRB = useful::DownCast<CRigidBody>(GetCollider());

	/* 衝撃波のみ特殊処理 */
	if (m_pShockWave)
	{
		// 衝撃波のコライダーをゴーストにキャスト
		CGhost* pShockwaveGhost = useful::DownCast<CGhost>(m_pShockWave->GetCollider());

		// オブジェクトのリストを取得
		const auto& rObjList = CObjectManager::RefInstance().RefObjList();

		for (const auto& rTypeList : rObjList)
		{
			for (const auto& rIt : rTypeList)
			{
				CPhysicsObject* pPhysicsObject = dynamic_cast<CPhysicsObject*>(rIt);

				// 物理オブジェクトにキャスト可能なら
				if (pPhysicsObject)
				{
					CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(pPhysicsObject->GetCollider());

					// 自分との判定は行わない
					if (pRB == pRigidBody)
					{
						continue;
					}

					// リジッドボディを持っていたら
					if (pRigidBody)
					{
						pRigidBody->SetActive();
						Collision::BumperPush(pShockwaveGhost, pRigidBody, 3.0f);
					}
				}
			}
		}
	}

	/*------------------------------------*/
	// これは死亡扱いのテスト

	// ワールドトランスフォームから位置を取得
	const DirectX::XMFLOAT3& Pos = pRB->GetWorldTransform().Pos;

	// フィールドの高さを下回ったら
	/* フィールドを参照すること */
	if (Pos.y < 3.0f)
	{
	   // 自身の死亡フラグを立てる
		SetDeath();
		CDust::GenerateSpread(Pos, 20);
	}
}

//============================================================================
// 塵の進行更新
//============================================================================
void CPlayer::UpdateDustStep(const DirectX::XMFLOAT3& Direction)
{
	++m_nStepCounter;

	if (m_nStepCounter > DUST_STEP_COUNT_MAX)
	{
		m_nStepCounter = 0;

		// 塵：直線発生：移動した場所の軌跡をなぞるように
		CDust::GenerateLinear(dynamic_cast<CRigidBody*>(GetCollider())->GetWorldTransform().Pos, Direction);
	}
}

//============================================================================
// 描画処理
//============================================================================
void CPlayer::Draw()
{
	// モデルの描画
	CPhysicsObject::Draw();
}

//============================================================================
// プレイヤーのインデックスを取得
//============================================================================
unsigned char CPlayer::GetIdxPlayer() const
{
	return m_wIdxPlayer;
}

//============================================================================
// プレイヤーのインデックスを設定
//============================================================================
void CPlayer::SetIdxPlayer(unsigned char wIdx)
{
	m_wIdxPlayer = wIdx;
}

//============================================================================
// 制御不能機関の設定
//============================================================================
int CPlayer::GetLostControlDuration() const
{
	return m_nLostControlDuration;
}

//============================================================================
// 制御不能機関の設定
//============================================================================
void CPlayer::SetLostControlDuration(int nTime)
{
	m_nLostControlDuration = nTime;
}