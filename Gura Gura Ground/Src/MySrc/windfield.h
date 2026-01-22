//============================================================================
// 
// 風ステージ[windfield.h]
// Auther:千葉
// 
//============================================================================

//============================================================================
//主にプレイヤーとCPUの処理を分けずそのまま関数でまとめてます
//============================================================================


//============================================================================
//インクルードガード
#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.physics.model.h"

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
	void SearchInfo();

	/**
	 * @brief 移動処理
	 * @param [in] 向き、移動速度、プレイヤーの人数、CPUの人数
	 */
	void MovePlayer(float Angle, float speed,int PlayerSize,int CPUSize);

	/**
	 * @brief 移動させる時に必要な処理群
	 * @param [in] リジットボディのポインター、向き、移動速度
	 */
	void ApplyWindToBody(CRigidBody* pRB, float Angle, float speed);

private:
	std::vector<std::shared_ptr<CPlayer>>m_pwPlayer;           //プレイヤーの閲覧用ポインター（複数人必要な為、vectorで管理）
	std::vector<std::shared_ptr<CEnemyPlayer>>m_pwEnemyPlayer; //敵プレイヤーの閲覧用ポインター（複数人必要な為、vectorで管理）
};