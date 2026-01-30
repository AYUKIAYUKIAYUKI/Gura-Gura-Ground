//============================================================================
// 
// 風ステージ[windfield.h]
// Auther:千葉
// 
//============================================================================


//============================================================================
//インクルードガード
#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.physics.model.h"
#include "API.hud.h"
#include <API.collision.h>      //btVector3の使用

#include <random>

//===================================================
//前方宣言
class CPlayer;
class CEnemyPlayer;
class CRigidBody;

//****************************************************
// フィールドクラスの定義
//****************************************************
class CWindField : public CPhysicsModel
{
private: //構造体

	//風ギミックに必要な情報群
	struct Parameter
	{
		float m_WindAngle = 0.0f;  //風の角度
		float m_WindSpeed = 1.0f;  //風の強さ

		float m_Timer = 0.0f;      //測定時間観測用
		float m_BlowTime = 60.0f;  //風が吹く時間
		float m_StopTime = 60.0f*5.0f; //風が止む時間

		bool m_IsBlowing = false;  //今風が吹いているか？
	};

public:

	//****************************************************
	// special function
	//****************************************************
	CWindField(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CWindField() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// コライダーのファクトリ
	void FactoryCollider(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f) override;

	/**
	 * @brief 更新処理
	 */
	void Update() override;

	/**
	 * @brief 描画処理
	 */
	void Draw() override;

private:
	/**
	 * @brief 全てのプレイヤー（CPU含むよ）の処理をまとめる用更新処理
	 */
	void UpdatePlayersSystem();

	/**
	 * @brief 情報を探す処理
	 */
	void SearchPYInfo();
	void SearchCPUInfo();

	/**
	 * @brief 移動処理
	 * @param [in] 向き、移動速度、プレイヤーの人数、CPUの人数
	 */
	void MovePlayer(float Angle, float speed, int PlayerSize, int CPUSize);

	/**
	 * @brief 移動させる時に必要な処理群
	 * @param [in] リジットボディのポインター、向き、移動速度
	 */
	void ApplyWindToBody_PY(CRigidBody* pRB, float Angle, float speed, std::weak_ptr<CPlayer> m_pwPlayer);

	/**
	 * @brief 移動させる時に必要な処理群
	 * @param [in] リジットボディのポインター、向き、移動速度
	 */
	void ApplyWindToBody_CPU(CRigidBody* pRB, float Angle, float speed, std::weak_ptr<CEnemyPlayer> pwCPU);

	/**
	 * @brief 共通する風の影響処理
	 * @param [in] リジットボディ、向き、移動速度、加速値
	 */
	void ApplyWindCommon(CRigidBody* pRB, float Angle, float speed);

	/**
	 * @brief 風の処理(強さなど)
	 */
	void Window();

	/**
	 * @brief 乱数
	 * [in] 最小値、最大値
	 */
	float RandomRange(float min, float max)
	{
		static std::mt19937 mt{ std::random_device{}() };
		std::uniform_real_distribution<float> dist(min, max);
		return dist(mt);
	}

	//============================================================================
// 着地判定：状態共通
//============================================================================
	template<class T>
	bool CheckLand(std::weak_ptr<T> t)
	{
		// プレイヤーのリジッドボディの取得
		CRigidBody* const pPlayerRigidBody = dynamic_cast<CRigidBody*>(t.lock()->GetCollider());

		//// 上昇中は着地判定を行わない
		//if (pPlayerRigidBody->GetLinearVelocity().getY() > 0.0f)
		//{
		//	return false;
		//}

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

private:
	Parameter m_Parameter;       //基本パラメータ
	btVector3 m_SaverCurrentVel; //取得した加速値を保存する用

	float m_WindowRotationAngle; //循環する風の角度を補完するよう(直接代入でもいいかも)

	std::vector<std::weak_ptr<CPlayer>>m_pwPlayer;           //プレイヤーの閲覧用ポインター（複数人必要な為、vectorで管理）
	std::vector<std::weak_ptr<CEnemyPlayer>>m_pwEnemyPlayer; //敵プレイヤーの閲覧用ポインター（複数人必要な為、vectorで管理）

	CHud* m_pWindArrow = nullptr;
	bool m_ShowArrow = false;   // ← 自前の表示フラグ
};