
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
struct btDefaultMotionState;
class  btRigidBody;

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
	CRigidBody(Collision::SHAPETYPE Type, float fWidth, float fHeight, float fDepth);

	// デストラクタ
	~CRigidBody() override;

	//****************************************************
	// function
	//****************************************************

	// 更新処理
	void Update(const OBJ::Transform& TF) override;

	// 更新処理
	void Draw() override;

	// モーションステートのユニークポインタを参照
	      std::unique_ptr<btDefaultMotionState>& UptrRefMotionState();
	const std::unique_ptr<btDefaultMotionState>& UptrRefMotionStateConst() const;

	// リジッドボディ本体のユニークポインタを参照
	      std::unique_ptr<btRigidBody>& UptrRefRigidBody();
	const std::unique_ptr<btRigidBody>& UptrRefRigidBodyConst() const;

	//****************************************************
	// static function
	//****************************************************

	// リジッドボディの生成
	static void CreateRigidBody(std::unique_ptr<CRigidBody>& upRB, Collision::SHAPETYPE Type = Collision::SHAPETYPE::BOX, float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);
	static void CreateRigidBody(std::unique_ptr<CCollider>&  upCL, Collision::SHAPETYPE Type = Collision::SHAPETYPE::BOX, float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);

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