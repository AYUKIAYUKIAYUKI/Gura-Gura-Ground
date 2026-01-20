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
#include "API.gltf.manager.h"
#include "API.input.manager.h"

// 当たり判定用
#include "API.collision.h"
#include "field.h"
#include "shockwave.h"

// 装飾用
#include "dust.h"
#include "shadow.h"

// カメラコントローラー登録解除のため
#include "cameracontroller.h"

// 静的メンバ初期化
std::vector<float> CPlayer::s_vSurvivalTimes(CPlayer::MAX_PLAYER_COUNT, 0.0f);

//****************************************************
// 無名名前空間の定義
//****************************************************
namespace
{
	// 条件制御
	int g_nStopCounter = 10;

	// 運動神経
	float g_fXZAxis_Speed = 9.0f;
	float g_fYAxis_Jump = 13.5f;

	// デバッグ用
	void DebugPrint(CPlayer& rPlayer)
	{
		// プレイヤーのインデックス取得
		const unsigned char wIdxPlayer = rPlayer.GetIdxPlayer();

		// 上の数値を文字列に変換
		const std::string WindowName = "Player Debug " + std::to_string(static_cast<int>(wIdxPlayer));

		// 現在のトランスフォームを取得
		const OBJ::Transform& TF = rPlayer.GetTransform();

		// プレイヤーのリジッドボディの取得
		CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(rPlayer.GetCollider());

		// 現在の加速度を取得
		const btVector3& rCurrentVel = pRigidBody->GetLinearVelocity();

		// 操作系統
		useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
		if (ImGui::TreeNodeEx(WindowName.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::DragInt("Stop Counter", &g_nStopCounter, 1);
			ImGui::Separator();

			// 現在の位置を表示
			ImGui::Text("PosX: %.2f", TF.Pos.x);
			ImGui::Text("PosY: %.2f", TF.Pos.y);
			ImGui::Text("PosZ: %.2f", TF.Pos.z);
			ImGui::Separator();

			// 現在の加速度を表示
			ImGui::Text("VelX: %.2f", rCurrentVel.getX());
			ImGui::Text("VelY: %.2f", rCurrentVel.getY());
			ImGui::Text("VelZ: %.2f", rCurrentVel.getZ());
			ImGui::Separator();

			ImGui::TreePop();
		}

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

	// 移動制御
	virtual void Move(CPlayer::StateMachine& rStateMachine, float fSpeed);

	// 接地判定
	virtual bool CheckLand(CPlayer::StateMachine& rStateMachine);
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
	CPlayer& m_rPalyer; // プレイヤーの参照
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
	bool      m_bGoDown = false;                // 下降判定
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
// 移動制御：状態共通
//============================================================================
void State::Move(CPlayer::StateMachine& rStateMachine, float fSpeedArg)
{
	// プレイヤーのリジッドボディの取得
	CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());

	// 現在の加速度をコピー
	const btVector3& rCurrentVel = pRigidBody->GetLinearVelocity();

	// 割り当てが該当するゲームパッドの方向入力を取得
	const std::optional<float>& opDirection = CInputManager::RefInstance().ConvertInputToMoveDirection(rStateMachine.m_rPalyer.GetIdxPlayer());

	// 方向入力があるなら
	if (opDirection)
	{
		// 移動速度スケール
		float fSpeed = fSpeedArg;

		//何かしらのデバフが有効なら移動速度に倍率を掛ける
		if (rStateMachine.m_rPalyer.GetFallTetraBehavior() != nullptr)
		{
			float Decay = rStateMachine.m_rPalyer.GetFallTetraBehavior()->GetDecayValue();
			fSpeed *= Decay;
		}

#if 0
		// 数値を先行して取得
		float fDirectionValue = opDirection.value();

		// 移動速度スケールの作成
		//const float fSpeed = fSpeedArg;
		btVector3   MoveDir = { 0.0f, 0.0f, 0.0f };

		// 移動方向：XZ軸：方向に沿って単位ベクトルに速度係数を掛けたものを設定
		// 　　　　：Y軸 ：現在の重力速度を維持
		MoveDir.setX(sinf(fDirectionValue));
		MoveDir.setY(0.0f);
		MoveDir.setZ(cosf(fDirectionValue));

		// 力を加える
		pRigidBody->SetActive();
		pRigidBody->SetForce(MoveDir * fSpeedArg);
#else
		// 数値を先行して取得
		float fDirectionValue = opDirection.value();

		// 移動速度スケールの作成
		//const float fSpeed = fSpeedArg;
		btVector3   MoveDir = { 0.0f, 0.0f, 0.0f };

		// 移動方向：XZ軸：方向に沿って単位ベクトルに速度係数を掛けたものを設定
		MoveDir.setX(sinf(fDirectionValue));
		MoveDir.setZ(cosf(fDirectionValue));

		// 目標の加速度作成
		const btVector3& TargetVel = MoveDir * fSpeed;

		/* ああ…btVector3をXMFLOAT3に変換 */
		DirectX::XMFLOAT3 CurrentVel_XMFLOAT = { rCurrentVel.getX(), 0.0f, rCurrentVel.getZ() };
		DirectX::XMFLOAT3 TargeVel_XMFLOAT   = { TargetVel.getX(),   0.0f, TargetVel.getZ() };

		/* ああ…要素ずつ指数減衰 */
		float fCoef = 0.25f;
		//何かしらのデバフが有効なら慣性に倍率を掛ける
		if (rStateMachine.m_rPalyer.GetFallTetraBehavior() != nullptr)	{
			float Inertia = rStateMachine.m_rPalyer.GetFallTetraBehavior()->GetInertiaValue();
			fCoef *= Inertia;
		}
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
#endif
	}
	else
	{
		btVector3 MoveDir = { 0.0f, 0.0f, 0.0f };

		// 加速度：XZ軸：値を抽出
		float fCurrentX = rCurrentVel.getX();
		float fCurrentZ = rCurrentVel.getZ();

		// 加速度：XZ軸：減衰をかける
		float fCoef = 0.05f;
		//何かしらのデバフが有効なら慣性に倍率を掛ける
		if (rStateMachine.m_rPalyer.GetFallTetraBehavior() != nullptr) {
			float Inertia = rStateMachine.m_rPalyer.GetFallTetraBehavior()->GetInertiaValue();
			fCoef *= Inertia;
		}
		useful::ExponentialDecay(fCurrentX, 0.0f, fCoef);
		useful::ExponentialDecay(fCurrentZ, 0.0f, fCoef);

		// 移動方向：XZ軸：減衰を反映
		// 　　　　：Y軸 ：現在の重力速度を維持
		MoveDir.setX(fCurrentX);
		MoveDir.setY(rCurrentVel.getY());
		MoveDir.setZ(fCurrentZ);

		// 新しい加速度として線形速度を設定
		pRigidBody->SetLinearVelocity(MoveDir);
	}
}

//============================================================================
// 着地判定：状態共通
//============================================================================
bool State::CheckLand(CPlayer::StateMachine& rStateMachine)
{
	// プレイヤーのリジッドボディの取得
	CRigidBody* const pPlayerRigidBody = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());

	// 上昇中は着地判定を行わない
	if (pPlayerRigidBody->GetLinearVelocity().getY() > 0.0f)
	{
		return false;
	}

	// 衝突判定の結果
	bool m_bHit = false;

	// 生ポインタのオブジェクトのリジッドボディと衝突判定
	if (Collision::CheckHitToRigidBodyRaw(pPlayerRigidBody))
	{
		m_bHit = true;
	}

	// シェアポインタのオブジェクトのリジッドボディと衝突判定
	if (Collision::CheckHitToRigidBodyShare(pPlayerRigidBody))
	{
		m_bHit = true;
	}

	return m_bHit;
}

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

	// プレイヤーのリジッドボディの取得
	CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());

	// 移動
	Move(rStateMachine, g_fXZAxis_Speed);

	// ジャンプ
	if (CInputManager::RefInstance().GetTrackerGamePad(rStateMachine.m_rPalyer.GetIdxPlayer()).a == DirectX::GamePad::ButtonStateTracker::PRESSED)
	{
		// ジャンプ力
		btVector3 btJumpVec = { 0.0f, g_fYAxis_Jump, 0.0f };

		// ジャンプ力を衝撃として加える
		pRigidBody->SetActive();
		pRigidBody->SetImpulse(btJumpVec);

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
	CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());

	// 現在の加速度をコピー
	const btVector3& rCurrentVel = pRigidBody->GetLinearVelocity();

	// 下降判定
	if (!m_bGoDown && rCurrentVel.getY() < 0.0f && m_btOldVel.getY() > 0.0f)
	{
		m_bGoDown = true;
	}

	// 下降判定取り消し
	if (m_bGoDown && rCurrentVel.getY() < -8.0f && m_btOldVel.getY() < -8.0f)
	{
		m_bGoDown = false;
	}

	// 移動
	Move(rStateMachine, g_fXZAxis_Speed);

	// 状態変更
	if (m_bGoDown && CInputManager::RefInstance().GetTrackerGamePad(rStateMachine.m_rPalyer.GetIdxPlayer()).a == DirectX::GamePad::ButtonStateTracker::PRESSED)
	{
		// キネマティック化
		pRigidBody->SetKinematic();

		// 下降判定後、ジャンプ状態中に追加入力でドロップ状態に変更
		rStateMachine.ChangeState(std::make_unique<StateDrop>());
	}
	else if (CheckLand(rStateMachine))
	{
		// 地面に着地していたら通常状態に変更
		rStateMachine.ChangeState(std::make_unique<StateDefault>());
	}

	// 現在の加速度情報を次フレームへ持ち越し
	m_btOldVel = rCurrentVel;
}

//============================================================================
// 状態実行：ドロップステート
//============================================================================
void StateDrop::Execute(CPlayer::StateMachine& rStateMachine)
{
	// 停止期間カウンターは常時動作
	++m_nStopCounter;

	// リジッドボディの取得
	CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());

	// 現在のワールドトランスフォームをコピー
	OBJ::Transform TF = pRigidBody->GetWorldTransform();

	if (m_nStopCounter < g_nStopCounter)
	{
		// 位置を少しずらす
		TF.Pos =
		{
		   TF.Pos.x + useful::GetRandomValue<float>() * 0.0005f,
		   TF.Pos.y + useful::GetRandomValue<float>() * 0.0005f,
		   TF.Pos.z + useful::GetRandomValue<float>() * 0.0005f
		};

		// トランスフォームをモーションステートに反映する
		pRigidBody->SetWorldTransform(TF);
	}
	else
	{
		// ドロップ力作成
		btVector3 btDropVec = { 0.0f, g_fYAxis_Jump * -2.0f, 0.0f };

		// ダイナミックに戻す
		pRigidBody->SetDynamic();

		// 下方向に衝撃を与える
		pRigidBody->SetActive();
		pRigidBody->SetImpulse(btDropVec);

		// 落下中に小さな球形の衝撃波を作成
		rStateMachine.m_rPalyer.CreateShockWave(Collision::SHAPETYPE::SPHERE, { 2.0f, 2.0f, 2.0f }, 1);
	}

	// 地面と接地したら
	if (CheckLand(rStateMachine))
	{
		// 衝撃波の作成
		rStateMachine.m_rPalyer.CreateShockWave(Collision::SHAPETYPE::CYLINDER, { 6.0f, 1.0f, 6.0f }, 10);

		// 通常状態に変更
		rStateMachine.ChangeState(std::make_unique<StateDefault>());

		// 塵：拡散発生
		CDust::GenerateSpread(rStateMachine.m_rPalyer.GetTransform().Pos, 7);
	}
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CPlayer::CPlayer(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CPhysicsModel(Type, Layer)
	, m_upStateMachine(std::make_unique<StateMachine>(*this))
	, m_wpField()
	, m_wIdxPlayer(0)
	, m_nLostControlDuration(0)
	, m_nStepCounter(0)
	, m_pDebuffBehavior(nullptr)
{
	// シェアポインタのオブジェクトリストの参照
	const std::list<std::shared_ptr<CObject>>& rFieldList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::FIELD);

	// フィールドの弱参照を設定
	m_wpField = std::dynamic_pointer_cast<CField>(rFieldList.front());

	// モデルのバインド
	SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Test"));

	m_bIsDead = false; //死亡しているかどうか
}

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
	const CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(GetCollider());

	// 重力の設定
	pRigidBody->SetGravity({ 0.0f, -25.0f, 0.0f });

	// 摩擦力を設定
	pRigidBody->SetFriction(1.0f);

	// Y軸以外の回転をロック
	pRigidBody->SetAngularFactor({ 0.0f, 0.0f, 0.0f });

	// 影の作成
	CShadow* pShadow = CObjectManager::CreateRaw<CShadow>(
		OBJ::TYPE::NONE,
		OBJ::LAYER::DEFAULT);

	// 影の追従対象として自身を設定
	pShadow->SetTrackTarget(shared_from_this());
}

//============================================================================
// 更新処理
//============================================================================
void CPlayer::Update()
{
	// 制御不能期間は常にデクリメント
	--m_nLostControlDuration;

	//生存時間計測
	if (!m_bIsDead) 
	{
		if (m_wIdxPlayer < s_vSurvivalTimes.size())
			s_vSurvivalTimes[m_wIdxPlayer] += 1.0f / 60.0f; // 60FPSで換算
	}

	// 状態実行
	if (m_upStateMachine)
	{
		m_upStateMachine->ExecuteState();
	}
	if (m_pDebuffBehavior != nullptr)
	{
		if (!m_pDebuffBehavior->GetTimer())
		{
			m_pDebuffBehavior.reset();
			m_pDebuffBehavior = nullptr;
		}
	}
	// 死亡判定
	CheckDeath();

	// WVP行列用定数バッファの更新
	CPhysicsModel::Update();

	/* デバッグ表示 */
	DebugPrint(*this);
}

//============================================================================
// 描画処理
//============================================================================
void CPlayer::Draw()
{
	// モデルの描画
	CPhysicsModel::Draw();
}

//============================================================================
// 衝撃波の作成
//============================================================================
void CPlayer::CreateShockWave(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3A& Size, int nDuration)
{
	// 衝撃波の作成と、弱参照の設定
	const std::shared_ptr<CShockWave>& spShockWave = CObjectManager::CreateShare<CShockWave>(
		OBJ::TYPE::NONE,
		OBJ::LAYER::DEFAULT);

	// プレイヤーのトランスフォームを出現位置に設定
	spShockWave->SetTransform(GetTransform());

	// ゴーストの作成
	spShockWave->FactoryCollider(Type, Size.x, Size.y, Size.z);

	// 自身を無視対象に設定
	//spShockWave->SetIgnore(WeakThis().lock());
	spShockWave->SetIgnore(shared_from_this());

	// 期間の設定
	spShockWave->SetDuration(nDuration);
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

//============================================================================
// 塵の進行更新
//============================================================================
void CPlayer::UpdateDustStep(const DirectX::XMFLOAT3& Direction)
{
	++m_nStepCounter;

	// 進行カウンターが既定値に到達したら
	if (m_nStepCounter > DUST_STEP_COUNT_MAX)
	{
		m_nStepCounter = 0;

		// コライダーをリジッドボディにキャスト
		if (CRigidBody* pRigidBody = dynamic_cast<CRigidBody*>(GetCollider()))
		{
			// 位置を取得
			const DirectX::XMFLOAT3& Pos = pRigidBody->GetWorldTransform().Pos;

			// 塵：直線発生：移動した場所の軌跡をなぞるように
			CDust::GenerateLinear(Pos, Direction);
		}
	}
}

//============================================================================
// 死亡チェック
//============================================================================
void CPlayer::CheckDeath()
{
	// トランスフォームから高さを取得
	float fSelfPosY = GetTransform().Pos.y;

	// フィールドの高さを保有
	float fFieldPosY = 0.0f;

	// フィールドの高さを取得
	if (std::shared_ptr<CField> spField = m_wpField.lock())
	{
		fFieldPosY = spField->GetTransform().Pos.y;
	}

	// Y座標がフィールドの高さを下回ったら
	if (fSelfPosY < fFieldPosY)
	{
		// 自身の死亡フラグを立てる
		m_bIsDead = true;
		SetDeath();
	}
}