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
#include "API.collider.h"

//****************************************************
// 物理オブジェクトクラスの定義
//****************************************************
class CPhysicsObject : public CObject
{
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

	// コライダーの生成
	virtual void FactoryCollider(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);
	virtual void FactoryCollider(const DirectX::XMFLOAT3& rSize);
	virtual void FactoryCollider(Collision::SHAPETYPE Type, float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);
	virtual void FactoryCollider(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3& rSize);

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// トランスフォームの操作用
	inline const OBJ::Transform& GetTransform() const                   { return m_Transform; }
	inline       void            SetTransform(const OBJ::Transform& TF) { m_Transform = TF; }

	// コライダーの操作用
	inline       CCollider* GetCollider()                     { return m_upCollider.get(); }
	//inline const CCollider* GetColliderConst() const          { return m_upCollider.get(); }
	inline       void       SetCollider(CCollider* pCollider) { m_upCollider.reset(pCollider); }

private:

	//****************************************************
	// function
	//****************************************************
	void ErrorCheck(); // エラー検知

	//****************************************************
	// data
	//****************************************************
	OBJ::Transform             m_Transform;  // トランスフォーム
	std::unique_ptr<CCollider> m_upCollider; // コライダー
};