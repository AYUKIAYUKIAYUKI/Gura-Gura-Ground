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

// 入力を取得するため
#include "API.input.manager.h"

// 初期化処理でXモデルを取得するため
//#include "X.manager.h"

#include "state.h"
#include "API.object.manager.h"
#include "API.world.h"
#include "API.rigidbody.h"
#include "API.gltf.manager.h"
//****************************************************
// usingディレクティブ
//****************************************************
using namespace useful;

//****************************************************
// 静的メンバ変数の定義 (非公開)
//****************************************************

// ▽ 最終的には定数となりますが、漸次的に編集可能な値とします

// 目標値への補間係数
float CPlayer::COEF_CORRECT_TARGET = 0.6f;

// 重力加速度
float CPlayer::COEF_GRAVITY = -0.15f;

// ジャンプ可能回数
int CPlayer::NUM_LEFT_JUMP = 1;

// ジャンプ力
float CPlayer::COEF_TRIGGER_JUMP = 2.75f;

// 移動速度
float CPlayer::COEF_MOVE_SPEED = 2.0f;
float CPlayer::COEF_MOVE_SPEED_AIR = 0.3f;

// 制動力
float CPlayer::COEF_BRAKING = 0.5f;

//半径
float CPlayer::RADIUS = 10.0f;
//****************************************************
// 静的メンバ変数の定義 (公開)
//****************************************************

// デフォルトのファクトリ
std::function<bool(CPlayer*)> CPlayer::s_fpDefaultFactory =
[](CPlayer* pPlayer) -> bool
{
	// 初期化処理
	pPlayer->Initialize();

	// ファクトリ
	pPlayer->Factory();

	// モデルの取得
	//auto pModel = useful::PtrCheck(CXManager::RefInstance().RefRegistry().BindAtKey("Player"), "Syokika Lamda no Naka Model Nai");

	//ステータスを取得
	//const JSON& Json = OpenJsonFileMaybeThrow("Data\\JSON\\PlayerStatus.json");
	//COEF_CORRECT_TARGET = Json["CORRECT_TARGET"];// 目標値への補間係数
	//COEF_GRAVITY = Json["GRAVITY"];		 // 重力加速度
	//COEF_TRIGGER_JUMP = Json["TRIGGER_JUMP"];	 // ジャンプ力
	//COEF_MOVE_SPEED = Json["MOVE_SPEED"];	 // 移動速度(地上)
	//COEF_MOVE_SPEED_AIR = Json["MOVE_SPEED_AIR"];// 移動速度(空中)
	//COEF_BRAKING = Json["BRAKING"];		 // 制動力

	// モデルの設定
	//pPlayer->SetModel(pModel);

	return true;
};

//============================================================================
// コンストラクタ
//============================================================================
CPlayer::CPlayer(OBJ::TYPE Type, OBJ::LAYER Layer)
	: CGltf(Type, Layer)
	, m_nFrame(0)
	//, m_nLeftNumJump(NUM_LEFT_JUMP)
	, m_Velocity(VEC3_ZERO_INIT)
	, m_vMichos()
	, m_stateMachine(new StateMachine<CPlayer>())
{}

//============================================================================
// デストラクタ
//============================================================================
CPlayer::~CPlayer()
{
	//ステイト情報がある時
	if (m_stateMachine != nullptr)
	{
		delete m_stateMachine;
		m_stateMachine = nullptr;
	}
}

//============================================================================
// ファクトリ
//============================================================================
void CPlayer::Factory()
{
	const OBJ::Transform TF =
	{
		useful::VEC3_ONE_INIT,
		{ 0.0f,  0.0f, 0.0f, 1.0f },
		{ 0.0f, 10.0f, 0.0f }
	};


	// プレイヤー用のリジッドボディの作成
	CRigidBody::CreateRigidBody(TF/*本来はファクトリでm_Transformを操作しGetTransform()など*/, UptrRefCollider(), Collision::SHAPETYPE::CYLINDER, 1.0f, 2.0f, 1.0f);
}

//============================================================================
//ジャンプ処理
//============================================================================
void CPlayer::Jump()
{
	btVector3   rMoveDir = { 0.0f, 0.0f, 0.0f };

	// コライダーからリジッドボディを取得
	const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(UptrRefColliderConst().get());

	if (!pRB)
	{
		return;
	}

	// 加速度：Y軸：現在の重力速度を維持
	btVector3 rCurrentVel = pRB->UptrRefRigidBodyConst()->getLinearVelocity();
	rMoveDir.setY(COEF_TRIGGER_JUMP);

	// 新しい速度を設定
	pRB->UptrRefRigidBodyConst()->setLinearVelocity(rMoveDir);

	// 加速度：XZ軸：速度を抑えつつ余韻を遺す
	// 　　　：Y軸 ：ジャンプ力を与える
	m_Velocity =
	{
		m_Velocity.x * COEF_MOVE_SPEED_AIR,
		COEF_TRIGGER_JUMP,
		m_Velocity.z * COEF_MOVE_SPEED_AIR
	};

	// みちょ数
	const int nMichosRepeat = 3;

	// みちょっと幅
	std::array<float, nMichosRepeat> afScales =
	{
		-0.25f, -0.35f, -0.15f
	};

	// みちょっとしてみる
	for (int i = 0; i < nMichosRepeat; ++i)
	{
		i % 2 == 0 ?
			m_vMichos.push_back({ 1.0f + afScales[i], 1.0f - afScales[i], 1.0f + afScales[i] }) :
			m_vMichos.push_back({ 1.0f - afScales[i], 1.0f + afScales[i], 1.0f - afScales[i] });
	}

	// 最後に元に戻す
	m_vMichos.push_back(VEC3_ONE_INIT);
}

//============================================================================
//ジャンプ中処理
//============================================================================
bool CPlayer::InJump()
{
	// 重力加速
	//bool bLanding = Gravity();

	// 地面に到達したら通常状態へ
	//if (bLanding)
	{
		// みちょ数
		const int nMichosRepeat = 5;

		// みちょっと幅
		std::array<float, nMichosRepeat> afScales =
		{
			0.25f, 0.20f, 0.15f, 0.10f, -0.15f
		};

		// みちょっとしてみる
		for (int i = 0; i < nMichosRepeat; ++i)
		{
			i % 2 == 0 ?
				m_vMichos.push_back({ 1.0f + afScales[i], 1.0f - afScales[i], 1.0f + afScales[i] }) :
				m_vMichos.push_back({ 1.0f - afScales[i], 1.0f + afScales[i], 1.0f - afScales[i] });
		}

		// 最後に元に戻す
		m_vMichos.push_back(VEC3_ONE_INIT);

		return true; //地面についた

	}

	return false;    //まだ地面じゃない
}


//============================================================================
//ダメージ処理
//============================================================================
void CPlayer::Damage()
{
	
}

//============================================================================
// 初期化処理
//============================================================================
bool CPlayer::Initialize()
{
	//// オブジェクト(Xモデル)の初期化処理
	//if (FAILED(CGltf::Initialize()))
	//{
	//	return false;
	//}

	m_stateMachine->Start(this);

	// 初期状態のステートをセット
	m_stateMachine->ChangeState<CPlayer_DefaultState>();

	return true;
}

//============================================================================
// 終了処理
//============================================================================
void CPlayer::Finalize()
{
	// オブジェクト(Xモデル)の終了処理
	//CGltf::Finalize();
}

//============================================================================
// 更新処理
//============================================================================
void CPlayer::Update()
{
	m_stateMachine->Update(); //ステートの更新処理

	// 数値編集
	ValueEdit();

	// 振動再生
	PlayWave();

	// オブジェクト(Xモデル)の更新処理
	// 行列の再計算を含んでいるため更新処理の終わりに呼びます
	CGltf::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CPlayer::Draw()
{
	// オブジェクト(Xモデル)の描画処理
	CGltf::Draw();
}

//============================================================================
// 移動
//============================================================================
void CPlayer::Move(float fSpeed)
{
	// 移動方向の取得：引数 0 ～ 3 ⇒ コントローラーの番号
	const std::optional<float>& opDir = CInputManager::RefInstance().ConvertInputToMoveDirection(0);

	// 入力があるなら
	if (opDir)
	{
		// 移動速度スケール
		const float fSpeed = 5.0f;
		btVector3   rMoveDir = { 0.0f, 0.0f, 0.0f };

		// 加速度：XZ軸：方向に沿って単位ベクトルに速度係数を掛けたものを設定
		rMoveDir.setX(sinf(opDir.value()) * fSpeed);
		rMoveDir.setZ(cosf(opDir.value()) * fSpeed);

		// コライダーからリジッドボディを取得
		const CRigidBody* const pRB = dynamic_cast<CRigidBody*>(UptrRefColliderConst().get());

		if (!pRB)
		{
			return;
		}

		// 加速度：Y軸：現在の重力速度を維持
		btVector3 rCurrentVel = pRB->UptrRefRigidBodyConst()->getLinearVelocity();
		rMoveDir.setY(rCurrentVel.getY());

		// 新しい速度を設定
		pRB->UptrRefRigidBodyConst()->setLinearVelocity(rMoveDir);
	}

	// 加速度：XZ軸：時間に伴い減衰していく
	ExponentialDecay(m_Velocity.x, 0.0f, COEF_BRAKING);
	ExponentialDecay(m_Velocity.z, 0.0f, COEF_BRAKING);
}

//============================================================================
// 移動
//============================================================================
bool CPlayer::JudgeInput()
{
	// 入力方向を取得
	std::optional<float> opDir = CInputManager::RefInstance().ConvertInputToMoveDirection();

	// 入力があるなら
	if (opDir)
	{
		return true;
	}

	return false;
}

//============================================================================
// 重力加速
//============================================================================
bool CPlayer::Gravity()
{
	//// 加速度：Y軸：下方向へ加速度を増加
	//m_Velocity.y += COEF_GRAVITY;

	//// このフレーム、地面に到達するなら
	//if (m_PosTarget.y + m_Velocity.y < 0.0f)
	//{
	//	// 加速度　：Y軸：停止
	//	// 目標位置：Y軸：地面に合わせる
	//	m_Velocity.y = 0.0f;
	//	m_PosTarget.y = 0.0f;

	//	return true;
	//}

	return false;
}

//============================================================================
// 振動再生
//============================================================================
void CPlayer::PlayWave()
{
	// 指定が何も無ければ処理しない
	if (m_vMichos.empty())
	{
		return;
	}

	// 先頭要素の参照
	const Vec3& front = m_vMichos.front();

	// 目標サイズ：みちょっとにする
	m_SizeTarget = front;

	// 現在のサイズを取得
	const Vec3& Size = GetTransform().Size;

	// 近似値を設定する
	const float fε = 0.1f;

	// 現在のサイズと目標サイズが十分近ければ
	if (fabsf(Size.x - m_SizeTarget.x) < fabsf(fε) &&
		fabsf(Size.y - m_SizeTarget.y) < fabsf(fε) &&
		fabsf(Size.z - m_SizeTarget.z) < fabsf(fε))
	{
		// 先頭要素を削除
		m_vMichos.pop_front();
	}
}

//============================================================================
// 攻撃当たった処理
//============================================================================
bool CPlayer::Hit()
{
	
	return true;
}

//============================================================================
// 数値編集
//============================================================================
void CPlayer::ValueEdit()
{
	// 静的メンバ変数を初期値として、全てコピー
	float fCoefCrorrectTarget = CPlayer::COEF_CORRECT_TARGET;
	float fCoefGravity = CPlayer::COEF_GRAVITY;
	float fCoefTriggerJump = CPlayer::COEF_TRIGGER_JUMP;
	float fCoefMoveSpeed = CPlayer::COEF_MOVE_SPEED;
	float fCoefMoveSpeedAir = CPlayer::COEF_MOVE_SPEED_AIR;
	float fCoefBraking = CPlayer::COEF_BRAKING;

	// 変化量
	const float fSpeed = 0.01f;

	// ImGuiで編集
	MIS::MyImGuiShortcut_BeginWindow(reinterpret_cast<const char*>(u8"プレイヤーの各種パラメータ操作"));
	{
		ImGui::DragFloat(reinterpret_cast<const char*>(u8"目標値への補間係数"), &fCoefCrorrectTarget, fSpeed, fSpeed, 1.0f);
		ImGui::DragFloat(reinterpret_cast<const char*>(u8"重力"), &fCoefGravity, fSpeed, FLT_MIN, 0.0f);
		ImGui::DragFloat(reinterpret_cast<const char*>(u8"ジャンプ力"), &fCoefTriggerJump, fSpeed, fSpeed, FLT_MAX);
		ImGui::DragFloat(reinterpret_cast<const char*>(u8"地上の移動速度"), &fCoefMoveSpeed, fSpeed, fSpeed, FLT_MAX);
		ImGui::DragFloat(reinterpret_cast<const char*>(u8"空中の移動速度"), &fCoefMoveSpeedAir, fSpeed, fSpeed, FLT_MAX);
		ImGui::DragFloat(reinterpret_cast<const char*>(u8"制動力"), &fCoefBraking, fSpeed, fSpeed, FLT_MAX);
		if (ImGui::Button(reinterpret_cast<const char*>(u8"書き出す")))
		{
			ExportStatus();
		}
		// サイズを出力
		const Vec3& Size = GetTransform().Size;
		ImGui::Text("Size:(%.2f, %.2f, %.2f)", Size.x, Size.y, Size.z);

		// 向きを出力
		const Vec4& Rot = GetTransform().Rot;
		ImGui::Text("Rot:(%.2f, %.2f, %.2f)", Rot.x, Rot.y, Rot.z);

		// 座標を出力
		const Vec3& Pos = GetTransform().Pos;
		ImGui::Text("Pos:(%.2f, %.2f, %.2f)", Pos.x, Pos.y, Pos.z);

		// 加速度を出力
		ImGui::Text("Velocity:(%.2f, %.2f, %.2f)", m_Velocity.x, m_Velocity.y, m_Velocity.z);

		// ステートを出力
		//ImGui::Text("State:%s", ToString());

	}
	ImGui::End();

	// 編集した値をメンバ変数に反映
	CPlayer::COEF_CORRECT_TARGET = fCoefCrorrectTarget;
	CPlayer::COEF_GRAVITY = fCoefGravity;
	CPlayer::COEF_TRIGGER_JUMP = fCoefTriggerJump;
	CPlayer::COEF_MOVE_SPEED = fCoefMoveSpeed;
	CPlayer::COEF_MOVE_SPEED_AIR = fCoefMoveSpeedAir;
	CPlayer::COEF_BRAKING = fCoefBraking;
}

//============================================================================
// 現在のステートをみたい
//============================================================================
const char* CPlayer::ToString()
{
	const char* StateName = m_stateMachine->GetState()->GetStateName();
	return StateName;
}

//============================================================================
// ステータスを書き出す
//============================================================================
void CPlayer::ExportStatus()
{
	//jsonファイルを作成
	std::ofstream writing_file;
	std::string filepath = "Data\\JSON\\PlayerStatus.json";
	writing_file.open(filepath, std::ios::out);

	JSON jsondata =
	{
		{"CORRECT_TARGET", COEF_CORRECT_TARGET},// 目標値への補間係数
		{"GRAVITY",COEF_GRAVITY },				// 重力加速度
		{"TRIGGER_JUMP",COEF_TRIGGER_JUMP },	// ジャンプ力
		{"MOVE_SPEED",COEF_MOVE_SPEED},			// 移動速度(地上)
		{"MOVE_SPEED_AIR",COEF_MOVE_SPEED_AIR},	// 移動速度(空中)
		{"BRAKING",COEF_BRAKING},				// 制動力
	};

	//作成したファイルに内容を書き込む
	writing_file << jsondata.dump(4);

	writing_file.close();

}

//============================================================================
// 半径を返す
//============================================================================
float CPlayer::GetRadius()
{
	return RADIUS;
}