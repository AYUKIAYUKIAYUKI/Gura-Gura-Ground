//============================================================================
// 
// 物理オブジェクト、ヘッダーファイル [physics.object.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.object.h"

//****************************************************
// 前方宣言
//****************************************************
class  CCollider;

//****************************************************
// 物理オブジェクトクラスの定義
//****************************************************
class CPhysicsObject : public CObject
{
	//****************************************************
	// 前方宣言
	//****************************************************
	struct Impl;

public:

	//****************************************************
	// special function
	//****************************************************

	// デフォルトコンストラクタ
	CPhysicsObject(OBJ::TYPE Type, OBJ::LAYER Layer);

	// デストラクタ
	~CPhysicsObject() override;

	//****************************************************
	// function
	//****************************************************

	// リジッドボディのファクトリ
	virtual void FactoryRigidBody(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);

	// ゴーストのファクトリ
	virtual void FactoryGhost(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// トランスフォームの操作用
	const OBJ::Transform& GetTransform() const;
	      void            SetTransform(const OBJ::Transform& TF);

	// コライダーのユニークポインタの参照
	      std::unique_ptr<CCollider>& UptrRefCollider();
	const std::unique_ptr<CCollider>& UptrRefColliderConst() const;

private:

	//****************************************************
	// function
	//****************************************************
	void ErrorCheck(); // エラー検知

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<Impl> m_upImpl;
};