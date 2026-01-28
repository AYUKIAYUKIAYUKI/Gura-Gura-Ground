//============================================================================
// 
// 鳥の群れ [BirdStrike.h]
// Author : 元地弘汰
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "obstacle.h"
#include "API.rigidbody.h"
#include "obstacle_editer.h"


//****************************************************
// 鳥の武重クラスの定義
//****************************************************
class CBirdStrike : public CObstacle
{
public:

	//****************************************************
	// special function
	//****************************************************
	CBirdStrike(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CBirdStrike() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// コライダーのファクトリ
	void FactoryCollider(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f) override;

	// 更新処理
	void Update() override;


	// 描画処理
	void Draw() override;

	// パラメータの編集
	void EditParam() override { int i = 0; }

	// 進行方向の取得
	inline DirectX::XMFLOAT3 GetDirection() const { return { m_Goal.x - m_Start.x, m_Goal.y - m_Start.y, m_Goal.z - m_Start.z }; }

private:

	//****************************************************
	// function
	//****************************************************

	//挙動
	void Action();

	//プレイヤーとの接触
	void ToPlayer();

	//****************************************************
	// data
	//****************************************************

	//補間の始点と終点
	DirectX::XMFLOAT3 m_Start;
	DirectX::XMFLOAT3 m_Goal;
	std::vector<DirectX::XMFLOAT3> m_Targets;

	//移動補間用時間
	int m_nTime;
};
