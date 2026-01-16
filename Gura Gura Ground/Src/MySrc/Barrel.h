//============================================================================
// 
// タル [Barrel.h]
// Author : 元地弘汰
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "obstacle.h"
#include "API.rigidbody.h"


//****************************************************
// タルクラスの定義
//****************************************************
class CBarrel : public CObstacle
{
public:

	//****************************************************
	// special function
	//****************************************************
	CBarrel(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CBarrel() override;                       // デストラクタ

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
	// 進行方向の設定
	inline const DirectX::XMFLOAT3& GetDirection() const { return m_Direction; }
	inline       void               SetDirection(const DirectX::XMFLOAT3& Direction) { m_Direction = Direction; }
	static void SetRotate(OBJ::Transform& rTF, DirectX::XMFLOAT3 Dir);


	void SetParamSetIndex(int idx) { m_ParamSetIndex = idx; }
	void SetSubParamIndex(int idx) { m_SubParamIndex = idx; }
	int  GetParamSetIndex() const { return m_ParamSetIndex; }
	int  GetSubParamIndex() const { return m_SubParamIndex; }
private:

	//****************************************************
	// function
	//****************************************************
	
	//挙動
	void Action();

	//****************************************************
	// data
	//****************************************************
	DirectX::XMFLOAT3 m_Direction; // 進行方向

	int m_ParamSetIndex = 0;   // どのParamSetか
	int m_SubParamIndex = 0;   // その中の何番目か

	int m_nTimer;				//特定行動の制御をするためのタイマー用変数
	float m_fHalfSize;			//オイルと半径を共有するための変数
};
