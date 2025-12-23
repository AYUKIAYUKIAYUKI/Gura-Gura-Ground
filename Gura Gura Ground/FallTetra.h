//============================================================================
// 
// ドッスン [FallTetra.h]
// Author : 元地弘汰
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "obstacle.h"

//前方宣言
class Tetra_State;

//****************************************************
// ドッスン(落下四面体)クラスの定義
//****************************************************
class CFallTetra : public CObstacle
{
public:

	//****************************************************
	// special function
	//****************************************************
	CFallTetra(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CFallTetra() override;                       // デストラクタ

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

	inline void ChangeState(std::shared_ptr<Tetra_State> NextState) {
		if (m_State != nullptr)NextState = nullptr;
		m_State = NextState;
	}
private:

	//****************************************************
	// function
	//****************************************************
	void Appear() {};


	//****************************************************
	// data
	//****************************************************
	std::shared_ptr<Tetra_State> m_State;

};

//****************************************************
// ドッスン(落下四面体)用ステート
//****************************************************
class Tetra_State
{		//ステート基底
public:
	virtual void Action([[maybe_unused]]CFallTetra* p) = 0;
};

class TetraState_Wait:public Tetra_State
{		//空中で待機ステート
public:
	TetraState_Wait(DirectX::XMFLOAT3 defaultposition) :m_Timer(0) {m_DefaultPos = defaultposition;}
	void Action([[maybe_unused]] CFallTetra* p)override;
private:
	DirectX::XMFLOAT3 m_DefaultPos;
	int m_Timer;
};

class TetraState_Fall :public Tetra_State
{		//落下状態ステート
public:
	TetraState_Fall(CFallTetra* p);
	void Action([[maybe_unused]] CFallTetra* p)override;
};