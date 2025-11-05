//========================================
//
//状態処理[state.cpp]
//Auther:Haruki Chiba
//
//========================================


//========================================
//インクルード
#include "state.h"           
#include "API.input.manager.h"   //キーボード処理
#include "API.object.manager.h"
//#include "impact.h"


//========================================
//名前空間
namespace PLAYER_INFO
{
	float f_Check_Velocity = 0.001f; //加速度の確認用変数
}


//==================================================================================================================================================
//基礎となるプレイヤーの状態処理
//==================================================================================================================================================

//========================================
//プレイヤージャンプ処理を呼ぶ処理
//========================================
bool CPlayerStateBase::CallJump(CPlayer* pPlayer)
{
	//SPACEキーが押された時かAボタンが押された時
	if (CInputManager::RefInstance().GetStateKeyboard().Space
		|| CInputManager::RefInstance().GetStateGamePad().IsAPressed()/*(CInputPad::JOYKEY::A)*/)
	{
		pPlayer->Jump(); //ジャンプ処理
		GetMacine()->ChangeState<CPlayer_JumpState>(); //ジャンプ状態へ移行

		return true;
	}

	return false;
}


//==================================================================================================================================================
//プレイヤーの通常状態処理
//==================================================================================================================================================

//========================================
//通常状態の一度だけ呼ばれる処理
//========================================
void CPlayer_DefaultState::OnStart(CPlayer* pPlayer)
{
	pPlayer->Null();
	m_bOneFlag = false; //フラグの初期化
	//GetMacine()->PopState();
}

//========================================
//通常状態の更新処理
//========================================
void CPlayer_DefaultState::OnUpdate(CPlayer* pPlayer)
{
	//加速度が、限りなく０に近くない時（ずっと処理が回らないように） NOTE:汚いがいったん代用
	if (pPlayer->GetVelocity().x > PLAYER_INFO::f_Check_Velocity || pPlayer->GetVelocity().x < -PLAYER_INFO::f_Check_Velocity
		|| pPlayer->GetVelocity().z > PLAYER_INFO::f_Check_Velocity || pPlayer->GetVelocity().z < -PLAYER_INFO::f_Check_Velocity)
	{
		pPlayer->Move(pPlayer->COEF_MOVE_SPEED); //移動処理 
	}
	else
	{
		//フラグがoff
		if (m_bOneFlag == false)
		{
			pPlayer->SetVelocity({ useful::VEC3_ZERO_INIT }); //加速度の初期化
			m_bOneFlag = true; //フラグon=初期化されるまでは通さない
		}
	}

	//移動キーが押された時
	if (pPlayer->JudgeInput() == true)
	{
		GetMacine()->ChangeState<CPlayer_MoveState>(); //移動状態へ移行
	}

	//ジャンプ処理
	else if (CallJump(pPlayer) == true)
	{

	}

	////Bキーが押された時
	//else if (CInputManager::RefInstance().GetKeyboard()->GetTrigger(DIK_B))
	//{
	//	GetMacine()->ChangeState<CPlayer_DamageState>();  //ダメージ状態へ移行
	//}

	////Tキーが押された時
	//else if (CInputManager::RefInstance().GetKeyboard()->GetTrigger(DIK_T))
	//{
	//	GetMacine()->ChangeState<CPlayer_OsiriState>();  //お尻状態へ移行
	//}
}

//========================================
//通常状態の一度だけ呼ばれる終了処理
//========================================
void CPlayer_DefaultState::OnExit(CPlayer* pPlayer)
{
	pPlayer->Null();
}

//========================================
//現在のステートを教えるよ
//========================================
const char* CPlayer_DefaultState::GetStateName()
{
	const char* NowState = "Default";
	return NowState;
}


//==================================================================================================================================================
//プレイヤーの移動状態処理
//==================================================================================================================================================

//========================================
//移動状態の一度だけ呼ばれる初期処理
//========================================
void CPlayer_MoveState::OnStart(CPlayer* pPlayer)
{
	pPlayer->Null();
}

//========================================
//移動状態の更新処理
//========================================
void CPlayer_MoveState::OnUpdate(CPlayer* pPlayer)
{
	pPlayer->Move(pPlayer->COEF_MOVE_SPEED); //移動処理 

	//移動キーが押されていない時
	if (pPlayer->JudgeInput() == false)
	{
		GetMacine()->ChangeState<CPlayer_DefaultState>(); //通常状態へ移行
		return;
	}

	//ジャンプ処理
	else if (CallJump(pPlayer) == true)
	{
		return;
	}
}

//========================================
//移動状態の一度だけ呼ばれる終了処理
//========================================
void CPlayer_MoveState::OnExit(CPlayer* pPlayer)
{
	pPlayer->Null();
}

//========================================
//現在のステートを教えるよ
//========================================
const char* CPlayer_MoveState::GetStateName()
{
	const char* NowState = "Move";
	return NowState;
}


//==================================================================================================================================================
//プレイヤーのお尻状態処理
//==================================================================================================================================================

//========================================
//お尻状態の一度だけ呼ばれる初期処理
//========================================
void CPlayer_OsiriState::OnStart(CPlayer* pPlayer)
{
	pPlayer->Null();
}

//========================================
//お尻状態の更新処理
//========================================
void CPlayer_OsiriState::OnUpdate(CPlayer* pPlayer)
{
	pPlayer->Null();
	//Yキーが押された時
	//if (CInputManager::RefInstance().GetKeyboard()->GetTrigger(DIK_Y))
	//{
	//	GetMacine()->ChangeState<CPlayer_DefaultState>();  //通常状態へ移行
	//}
}

//========================================
//お尻状態の一度だけ呼ばれる終了処理
//========================================
void CPlayer_OsiriState::OnExit(CPlayer* pPlayer)
{
	pPlayer->Null();
}

//========================================
//現在のステートを教えるよ
//========================================
const char* CPlayer_OsiriState::GetStateName()
{
	const char* NowState = "HipTime";
	return NowState;
}


//==================================================================================================================================================
//プレイヤーのジャンプ状態処理
//==================================================================================================================================================

//========================================
//ジャンプ状態の一度だけ呼ばれる初期処理
//========================================
void CPlayer_JumpState::OnStart(CPlayer* pPlayer)
{
	pPlayer->Null();
}

//========================================
//ジャンプ状態の更新処理
//========================================
void CPlayer_JumpState::OnUpdate(CPlayer* pPlayer)
{
	pPlayer->Move(pPlayer->COEF_MOVE_SPEED); //移動処理 

	//地面についた時(処理を完結)
	if (pPlayer->InJump() == true)
	{
		////衝撃波生成
		//using namespace OBJ;
		//auto pImpact = CObject::Create<CImpact>(TYPE::IMPUCT, LAYER::DEFAULT, CImpact::s_fpDefaultFactory);
		//pImpact->SetPos(pPlayer->GetPos());			//位置を保存
		//pImpact->SetCreator(pPlayer);				//発生させた者を保存
		//pImpact->SetRadius(pPlayer->GetRadius());	//半径を保存
		//pImpact->Start();

		//プレイヤーが格納されているリストを取得
		std::list<CObject*> playerlist = CObjectManager::RefInstance().RefObjList(OBJ::TYPE::PLAYER);

		//コライダーの更新処理
		for (auto other : playerlist)
		{
			if (other == pPlayer)
			{//発生させた者ではない
				continue;
			}

		}

		//ダメージ処理
		pPlayer->Damage();

		GetMacine()->ChangeState<CPlayer_DefaultState>(); //通常状態へ移行
		return;
	}
}

//========================================
//ジャンプ状態の一度だけ呼ばれる終了処理
//========================================
void CPlayer_JumpState::OnExit(CPlayer* pPlayer)
{
	pPlayer->Null();
}

//========================================
//現在のステートを教えるよ
//========================================
const char* CPlayer_JumpState::GetStateName()
{
	const char* NowState = "Jump";
	return NowState;
}


//==================================================================================================================================================
//プレイヤーのダメージ状態処理
//==================================================================================================================================================

//========================================
//ジャンプ状態の一度だけ呼ばれる初期処理
//========================================
void CPlayer_DamageState::OnStart(CPlayer* pPlayer)
{
	pPlayer->SetVelocity({ useful::VEC3_ZERO_INIT }); //加速度の初期化(動かないようにする)
}

//========================================
//ダメージ状態の更新処理
//========================================
void CPlayer_DamageState::OnUpdate(CPlayer* pPlayer)
{
	nCount++;

	if (nCount > 120)
	{
		pPlayer->Damage(); //ダメージ処理

		GetMacine()->ChangeState<CPlayer_DefaultState>();     //通常状態へ移行
	}

	return;
}

//========================================
//ジャンプ状態の一度だけ呼ばれる終了処理
//========================================
void CPlayer_DamageState::OnExit(CPlayer* pPlayer)
{
	pPlayer->Null();
}

//========================================
//現在のステートを教えるよ
//========================================
const char* CPlayer_DamageState::GetStateName()
{
	const char* NowState = "Damage";
	return NowState;
}