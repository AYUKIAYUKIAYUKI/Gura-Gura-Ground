//============================================================================
// 
// ボム、ヘッダーファイル [bomb.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "obstacle.h"
#include "obstacle_editer.h"

//****************************************************
// 前方宣言
//****************************************************
class CBombShockWave;

//****************************************************
// ボールクラスの定義
//****************************************************
class CBomb : public CObstacle
{
public:

	//****************************************************
	// special function
	//****************************************************
	CBomb(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CBomb() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// コライダーのファクトリ
	void FactoryCollider(float fWidth, float fHeight, float fDepth) override;

	// 衝撃波の作成
	void CreateShockWave(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3& Size, int nDuration);

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// インスペクターの表示
	void ShowInspector() override;

	// パラメータの編集
	void EditParam() override;

	// タイマーの操作用
	inline int  GetTimer() const     { return m_nTimer; }
	inline void SetTimer(int nTimer) { m_nTimer = nTimer; }

	void SetParamSetIndex(int idx) { m_ParamSetIndex = idx; }
	void SetSubParamIndex(int idx) { m_SubParamIndex = idx; }
	int  GetParamSetIndex() const { return m_ParamSetIndex; }
	int  GetSubParamIndex() const { return m_SubParamIndex; }

private:

	//****************************************************
	// function
	//****************************************************
	void Action(); // 挙動

	//****************************************************
	// data
	//****************************************************
	int m_nTimer; // タイマー

	int m_ParamSetIndex = 0;   // どのParamSetか
	int m_SubParamIndex = 0;   // その中の何番目か
	ObstacleEditer m_ObstacleEditer;
};