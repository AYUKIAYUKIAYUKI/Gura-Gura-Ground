//============================================================================
// 
// 障害物、ヘッダーファイル [obstacle.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.physics.object.h"

//****************************************************
// 名前空間OBSを定義
//****************************************************
namespace OBS
{
	//****************************************************
	// タイプ識別用の列挙型を定義
	//****************************************************
	enum class OBSTACLE_TYPE : unsigned char
	{
		NONE = 0,	  // 無し
		MOVING,       // 一定方向に移動する
		STATIONARY,   // 定点に留まる
		PERIMETER,    // 外周を移動する
		MAX
	};
}

//****************************************************
// 障害物クラスの定義
//****************************************************
class CObstacle : public CPhysicsObject
{
public:

	//****************************************************
	// special function
	//****************************************************
	CObstacle(OBJ::TYPE Type, OBJ::LAYER Layer, OBS::OBSTACLE_TYPE ObsType); // デフォルトコンストラクタ
	~CObstacle() override;                       // デストラクタ

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
	virtual void EditParam() = 0;

	// 種類取得
	inline OBS::OBSTACLE_TYPE GetObsType() const { return m_ObsType; }
private:
	OBS::OBSTACLE_TYPE m_ObsType;	//　ギミックの種類
};