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
// 名前空間Obstacleを定義
//****************************************************
namespace Obstacle
{
	//****************************************************
	// 挙動識別用の列挙型を定義
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

	// デフォルトコンストラクタ
	CObstacle(OBJ::TYPE Type, OBJ::LAYER Layer, Obstacle::OBSTACLE_TYPE ObstacleType);
	
	// デストラクタ	
	~CObstacle() override;

	//****************************************************
	// function
	//****************************************************

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// インスペクターの表示
	virtual void ShowInspector();

	// パラメータの編集
	virtual void EditParam();

	// 選択中の障害物をバインド
	static void BindPickingObstacle(CObstacle* pObstacle);

	// 選択中の障害物の更新
	static void UpdatePickingObstacle();

	// 挙動の種類取得
	inline Obstacle::OBSTACLE_TYPE GetObsType() const { return m_ObsType; }

private:

	//****************************************************
	// data
	//****************************************************
	Obstacle::OBSTACLE_TYPE m_ObsType; // ギミックの種類

	// 選択中の障害物
	static CObstacle* s_pPickingObstacle;
};