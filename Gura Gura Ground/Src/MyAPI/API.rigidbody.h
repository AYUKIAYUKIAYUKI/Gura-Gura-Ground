
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// リジッドボディ、ヘッダーファイル [rigidbody.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.collider.h"

//****************************************************
// 前方宣言
//****************************************************
class btVector3;
class btRigidBody;

//****************************************************
// リジッドボディクラスの定義
//****************************************************
class CRigidBody : public CCollider
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
	CRigidBody(const OBJ::Transform& TF, Collision::SHAPETYPE Type, float fWidth, float fHeight, float fDepth);

	// デストラクタ
	~CRigidBody() override;

	//****************************************************
	// function
	//****************************************************

	// 更新処理
	void Update(const OBJ::Transform& TF) override;

	// 描画処理
	void Draw() override;

	// キネマティック化
	void SetKinematic() const;

	// ダイナミック化
	void SetDynamic() const;

	// アクティブ状態の操作用
	bool GetActive() const;
	void SetActive() const;

	// 質量の設定
	void SetMass(float fMass) const;

	// 弾性力の設定
	void SetRestitution(float fMass) const;

	// 摩擦力の設定
	void SetFriction(float fFriction)        const;
	void SetRollingFriction(float fFriction) const;

	// 使用する回転軸の設定
	void SetAngularFactor(const btVector3& Vec) const;

	// ワールドトランスフォームの操作
	OBJ::Transform GetWorldTransform()                                 const;
	void           GetWorldTransform(OBJ::Transform& rTransform)       const;
	void           SetWorldTransform(const OBJ::Transform& rTransform) const;

	// トルク回転の付与
	void SetTorqueImpulse(const btVector3& Impulse) const;

	// 線形加速度の操作
	const btVector3& GetLinearVelocity()                           const;
	void             SetLinearVelocity(const btVector3& rVelocity) const;

	// インパルスの付与
	void SetImpulse(const btVector3& Impulse) const;

	// リジッドボディ本体の取得
	      btRigidBody* GetRigidBody();
	const btRigidBody* GetRigidBodyConst() const;

	//****************************************************
	// static function
	//****************************************************

	// リジッドボディの生成
	static CCollider* CreateRigidBody(const OBJ::Transform& Transform, Collision::SHAPETYPE Type = Collision::SHAPETYPE::BOX, float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);

private:

	//****************************************************
	// function
	//****************************************************
	void Error(); // エラー処理

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<Impl> m_upImpl;
};