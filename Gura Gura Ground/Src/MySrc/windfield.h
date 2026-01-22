//============================================================================
// 
// 風ステージ[windfield.h]
// Auther:千葉
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.physics.model.h"

//===================================================
//前方宣言
class CPlayer;
class CEnemyPlayer;

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

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

private:
	void SearchInfo();

private:
	std::vector<std::shared_ptr<CPlayer>>m_pwPlayer;           //プレイヤーの閲覧用ポインター（複数人必要な為、vectorで管理）
	std::vector<std::shared_ptr<CEnemyPlayer>>m_pwEnemyPlayer; //敵プレイヤーの閲覧用ポインター（複数人必要な為、vectorで管理）
};