////============================================================================
//// 
//// ウソのプレイヤー [player.fake.cpp]
//// Author : 福田歩希
//// 
////============================================================================
//
////****************************************************
//// インクルードファイル
////****************************************************
//#include "player.fake.h"
//#include "API.gltf.manager.h"
//#include "API.input.manager.h"
//#include "effect.manager.h"
//#include "API.sound.manager.h"
//
//// 装飾用
//#include "dust.h"
//
///* 必要 */
//#include "API.renderer.h"
//#include "API.collision.h"
//#include "API.rigidbody.h"
//
////****************************************************
//// 無名名前空間の定義
////****************************************************
//namespace
//{
//	// 条件制御
//	int g_nStopCounter = 10;
//
//	// 運動神経
//	float g_fXZAxis_Speed = 9.0f;
//	float g_fYAxis_Jump = 13.5f;
//}
//
////****************************************************
//// ステート構造体の定義
////****************************************************
//struct State
//{
//	//****************************************************
//	// special funciton
//	//****************************************************
//	virtual ~State() = default;
//
//	//****************************************************
//	// funciton
//	//****************************************************
//
//	// 更新処理
//	virtual void Execute(CPlayerFake::StateMachine& rStateMachine) = 0;
//
//	// 移動制御
//	virtual void Move(CPlayerFake::StateMachine& rStateMachine, float fSpeed);
//
//	// 接地判定
//	virtual bool CheckLand(CPlayerFake::StateMachine& rStateMachine);
//};
//
////****************************************************
//// ステートマシン構造体の定義
////****************************************************
//struct CPlayerFake::StateMachine
//{
//	//****************************************************
//	// special funciton
//	//****************************************************
//	StateMachine(CPlayerFake& rPlayer); // デフォルトコンストラクタ
//
//	//****************************************************
//	// funciton
//	//****************************************************
//	void ExecuteState();                                   // 状態実行
//	void ChangeState(std::unique_ptr<State>&& upNewState); // 状態変更
//
//	//****************************************************
//	// data
//	//****************************************************
//	std::unique_ptr<State> m_upState; // 状態
//	CPlayerFake&           m_rPalyer; // プレイヤーの参照
//};
//
////****************************************************
//// デフォルトステート構造体の定義
////****************************************************
//struct StateDefault : public State
//{
//	//****************************************************
//	// funciton
//	//****************************************************
//
//	// 更新処理
//	void Execute(CPlayerFake::StateMachine& rStateMachine) override;
//};
//
////****************************************************
//// ジャンプステート構造体の定義
////****************************************************
//struct StateJump : public State
//{
//	//****************************************************
//	// funciton
//	//****************************************************
//
//	// 更新処理
//	void Execute(CPlayerFake::StateMachine& rStateMachine) override;
//
//	//****************************************************
//	// Data
//	//****************************************************
//	bool      m_bGoDown  = false;                // 下降判定
//	btVector3 m_btOldVel = { 0.0f, 0.0f, 0.0f }; // 過去の加速度
//};
//
////****************************************************
//// ドロップステート構造体の定義
////****************************************************
//struct StateDrop : public State
//{
//	//****************************************************
//	// funciton
//	//****************************************************
//
//	// 更新処理
//	void Execute(CPlayerFake::StateMachine& rStateMachine) override;
//
//	//****************************************************
//	// Data
//	//****************************************************
//	int m_nStopCounter = 0; // 停止期間カウンター
//};
//
////============================================================================
//// 移動制御：状態共通
////============================================================================
//void State::Move(CPlayerFake::StateMachine& rStateMachine, float fSpeedArg)
//{
//	// プレイヤーのリジッドボディの取得
//	CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());
//
//	// 現在の加速度をコピー
//	const btVector3& rCurrentVel = pRigidBody->GetLinearVelocity();
//
//	// 割り当てが該当するゲームパッドの方向入力を取得
//	const std::optional<float>& opDirection = CInputManager::RefInstance().ConvertInputToMoveDirection(rStateMachine.m_rPalyer.GetIdxPlayer());
//
//	// 方向入力があるなら
//	if (opDirection)
//	{
//
//	}
//}
//
////============================================================================
//// 着地判定：状態共通
////============================================================================
//bool State::CheckLand(CPlayerFake::StateMachine& rStateMachine)
//{
//	// プレイヤーのリジッドボディの取得
//	CRigidBody* const pPlayerRigidBody = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());
//
//	// 上昇中は着地判定を行わない
//	if (pPlayerRigidBody->GetLinearVelocity().getY() > 0.0f)
//	{
//		return false;
//	}
//
//	// 衝突判定の結果
//	bool m_bHit = false;
//
//	// 生ポインタのオブジェクトのリジッドボディと衝突判定
//	if (Collision::CheckHitToRigidBodyRaw(pPlayerRigidBody))
//	{
//		m_bHit = true;
//	}
//
//	// シェアポインタのオブジェクトのリジッドボディと衝突判定
//	if (Collision::CheckHitToRigidBodyShare(pPlayerRigidBody))
//	{
//		m_bHit = true;
//	}
//
//	return m_bHit;
//}
//
////============================================================================
//// デフォルトコンストラクタ：ステートマシン
////============================================================================
//CPlayerFake::StateMachine::StateMachine(CPlayerFake& rPlayer)
//	: m_upState(std::make_unique<StateDefault>())
//	, m_rPalyer(rPlayer)
//{}
//
////============================================================================
//// 状態実行：ステートマシン
////============================================================================
//void CPlayerFake::StateMachine::ExecuteState()
//{
//	if (m_upState)
//	{
//		m_upState->Execute(*this);
//	}
//}
//
////============================================================================
//// 状態変更：ステートマシン
////============================================================================
//void CPlayerFake::StateMachine::ChangeState(std::unique_ptr<State>&& upNewState)
//{
//	if (!upNewState)
//	{
//		return;
//	}
//
//	m_upState = std::move(upNewState);
//}
//
////============================================================================
//// 状態実行：デフォルトステート
////============================================================================
//void StateDefault::Execute(CPlayerFake::StateMachine& rStateMachine)
//{
//	// プレイヤーのリジッドボディの取得
//	CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());
//
//	// 移動
//	Move(rStateMachine, g_fXZAxis_Speed);
//
//	// ジャンプ
//	if (CInputManager::RefInstance().GetTrackerGamePad(rStateMachine.m_rPalyer.GetIdxPlayer()).a == DirectX::GamePad::ButtonStateTracker::PRESSED)
//	{
//		// ジャンプ力
//		btVector3 btJumpVec = { 0.0f, g_fYAxis_Jump, 0.0f };
//
//		CSoundManger::RefInstance().Play("Jump", false, 0.0f, 1.0f);
//
//		// ジャンプ力を衝撃として加える
//		pRigidBody->SetActive();
//		pRigidBody->SetImpulse(btJumpVec);
//
//		// ジャンプ状態に変更
//		rStateMachine.ChangeState(std::make_unique<StateJump>());
//	}
//}
//
////============================================================================
//// 状態実行：ジャンプステート
////============================================================================
//void StateJump::Execute(CPlayerFake::StateMachine& rStateMachine)
//{
//	// リジッドボディの取得
//	CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());
//
//	// 現在の加速度をコピー
//	const btVector3& rCurrentVel = pRigidBody->GetLinearVelocity();
//
//	// 下降判定
//	if (!m_bGoDown && rCurrentVel.getY() < 0.0f && m_btOldVel.getY() > 0.0f)
//	{
//		m_bGoDown = true;
//	}
//
//	// 下降判定取り消し
//	if (m_bGoDown && rCurrentVel.getY() < -8.0f && m_btOldVel.getY() < -8.0f)
//	{
//		m_bGoDown = false;
//	}
//
//	// 移動
//	Move(rStateMachine, g_fXZAxis_Speed);
//
//	// 状態変更
//	if (m_bGoDown && CInputManager::RefInstance().GetTrackerGamePad(rStateMachine.m_rPalyer.GetIdxPlayer()).a == DirectX::GamePad::ButtonStateTracker::PRESSED)
//	{
//		// キネマティック化
//		pRigidBody->SetKinematic();
//
//		// 下降判定後、ジャンプ状態中に追加入力でドロップ状態に変更
//		rStateMachine.ChangeState(std::make_unique<StateDrop>());
//	}
//	else if (CheckLand(rStateMachine))
//	{
//		// 地面に着地していたら通常状態に変更
//		rStateMachine.ChangeState(std::make_unique<StateDefault>());
//	}
//
//	// 現在の加速度情報を次フレームへ持ち越し
//	m_btOldVel = rCurrentVel;
//}
//
////============================================================================
//// 状態実行：ドロップステート
////============================================================================
//void StateDrop::Execute(CPlayerFake::StateMachine& rStateMachine)
//{
//	// 停止期間カウンターは常時動作
//	++m_nStopCounter;
//
//	// リジッドボディの取得
//	CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(rStateMachine.m_rPalyer.GetCollider());
//
//	// 現在のワールドトランスフォームをコピー
//	OBJ::Transform TF = pRigidBody->GetWorldTransform();
//
//	if (m_nStopCounter < g_nStopCounter)
//	{
//		// 位置を少しずらす
//		TF.Pos =
//		{
//		   TF.Pos.x + useful::GetRandomValue<float>() * 0.0005f,
//		   TF.Pos.y + useful::GetRandomValue<float>() * 0.0005f,
//		   TF.Pos.z + useful::GetRandomValue<float>() * 0.0005f
//		};
//
//		// トランスフォームをモーションステートに反映する
//		pRigidBody->SetWorldTransform(TF);
//	}
//	else
//	{
//		// ドロップ力作成
//		btVector3 btDropVec = { 0.0f, g_fYAxis_Jump * -2.0f, 0.0f };
//
//		// ダイナミックに戻す
//		pRigidBody->SetDynamic();
//
//		// 下方向に衝撃を与える
//		pRigidBody->SetActive();
//		pRigidBody->SetImpulse(btDropVec);
//	}
//
//	// 地面と接地したら
//	if (CheckLand(rStateMachine))
//	{
//		// ダイナミックに戻す
//		pRigidBody->SetDynamic();
//
//		useful::Vec3 EffectVec3 = { rStateMachine.m_rPalyer.GetTransform().Pos.x,6.25f,rStateMachine.m_rPalyer.GetTransform().Pos.z };
//		CEffect::Create(CEffectManager::EFFECT_TAG::TAG_HIPDROP, EffectVec3, nullptr, 1.6f);
//
//		//ドロップサウンド再生
//		CSoundManger::RefInstance().Play("Drop", false, 0.0f, 1.0f);
//
//		// 通常状態に変更
//		rStateMachine.ChangeState(std::make_unique<StateDefault>());
//
//		// 塵：拡散発生
//		CDust::GenerateSpread(rStateMachine.m_rPalyer.GetTransform().Pos, 7);
//	}
//}
//
////============================================================================
//// デフォルトコンストラクタ
////============================================================================
//CPlayerFake::CPlayerFake(OBJ::TYPE Type, OBJ::LAYER Layer)
//	: CPhysicsModel(Type, Layer)
//	, m_upStateMachine(std::make_unique<StateMachine>(*this))
//	, m_wIdxPlayer(0)
//	, m_nStepCounter(0)
//{
//	// モデルのバインド
//	SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Player_1"));
//	SetModelOffset({ 0.0f, -0.5f, 0.0f });
//
//	// 回転同期の解除
//	DisableSyncRotation();
//
//	// ピクセルシェーダ―のバインド
//	SetPixelShader(CPixelShaderManager::RefInstance().RefRegistry().BindAtKey("Ray.Marching"));
//}
//
////============================================================================
//// デストラクタ
////============================================================================
//CPlayerFake::~CPlayerFake()
//{}
//
////============================================================================
//// コライダーのファクトリ
////============================================================================
//void CPlayerFake::FactoryCollider(float fWidth, float fHeight, float fDepth)
//{
//	// プレイヤー用のリジッドボディの作成
//	SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::BOX, fWidth, fHeight, fDepth));
//
//	// コライダーをリジッドボディにキャスト
//	const CRigidBody* const pRigidBody = dynamic_cast<CRigidBody*>(GetCollider());
//
//	// 重力の設定
//	pRigidBody->SetGravity({ 0.0f, -25.0f, 0.0f });
//
//	// 摩擦力を設定
//	pRigidBody->SetFriction(1.0f);
//
//	// 減衰力を設定
//	pRigidBody->SetDamping(0.3f, 0.0f);
//
//	// Y軸以外の回転をロック
//	pRigidBody->SetAngularFactor({ 0.0f, 0.0f, 0.0f });
//}
//
////============================================================================
//// 更新処理
////============================================================================
//void CPlayerFake::Update()
//{
//	// 状態実行
//	if (m_upStateMachine)
//	{
//		m_upStateMachine->ExecuteState();
//	}
//
//	// WVP行列用定数バッファの更新
//	CPhysicsModel::Update();
//}
//
////============================================================================
//// 描画処理
////============================================================================
//void CPlayerFake::Draw()
//{
//	// モデルの描画
//	CPhysicsModel::Draw();
//}
//
////============================================================================
//// プレイヤーのインデックスを取得
////============================================================================
//unsigned char CPlayerFake::GetIdxPlayer() const
//{
//	return m_wIdxPlayer;
//}
//
////============================================================================
//// プレイヤーのインデックスを設定
////============================================================================
//void CPlayerFake::SetIdxPlayer(unsigned char wIdx)
//{
//	m_wIdxPlayer = wIdx;
//}