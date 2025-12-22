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
#include "API.rigidbody.h"

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

	void ToSmash();

	// 描画処理
	void Draw() override;

	// パラメータの編集
	void EditParam() override { int i = 0; }

	void ChangeState(std::shared_ptr<Tetra_State> NextState);

	//初期設定のゲッター
	inline DirectX::XMFLOAT3 GetInitalGravity() { return m_InitalGravity; }
	inline DirectX::XMFLOAT3 GetInitalPosition() { return m_InitalPosition; }

private:

	//****************************************************
	// function
	//****************************************************


	//****************************************************
	// data
	//****************************************************
	std::shared_ptr<Tetra_State> m_State;	//状態遷移用のステート
	DirectX::XMFLOAT3 m_InitalPosition;		//振動実装用、生成時の原点を保存用
	DirectX::XMFLOAT3 m_InitalGravity;		//落下までの猶予実装用、生成時の重力保存用
};

//****************************************************
// ドッスン(落下四面体)用ステート
//****************************************************
class Tetra_State
{		//ステート基底
public:
	Tetra_State([[maybe_unused]] CFallTetra* p = nullptr) {};
	virtual void Action([[maybe_unused]]CFallTetra* p) = 0;
};

class TetraState_Wait:public Tetra_State
{		//空中で待機ステート
public:
	TetraState_Wait([[maybe_unused]] CFallTetra* p);
	void Action([[maybe_unused]] CFallTetra* p)override;
private:
	DirectX::XMFLOAT3 m_DefaultPos;	//揺らすための初期位置
	int m_Timer;					//落下までの猶予時間
};

class TetraState_Fall :public Tetra_State
{		//落下状態ステート
public:
	TetraState_Fall([[maybe_unused]] CFallTetra* p);
	void Action([[maybe_unused]] CFallTetra* p)override;
private:
	int m_Grace;					//即消えないようにするための猶予
};