
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// ゴースト、ヘッダーファイル [ghost.h]
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
class btRigidBody;

//****************************************************
// ゴーストクラスの定義
//****************************************************
class CGhost : public CCollider
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
	CGhost(const OBJ::Transform& TF, Collision::SHAPETYPE Type, float fWidth, float fHeight, float fDepth);

	// デストラクタ
	~CGhost();

	//****************************************************
	// function
	//****************************************************

	// 更新処理
	void Update(const OBJ::Transform& TF) override;

	// 描画処理
	void Draw() override;

	// ワールドトランスフォームの操作
	OBJ::Transform GetWorldTransform()                                 const;
	void           GetWorldTransform(OBJ::Transform& rTransform)       const;
	void           SetWorldTransform(const OBJ::Transform& rTransform) const;

	// ゴースト本体の取得
	      btRigidBody* GetGhost();
	const btRigidBody* GetGhostConst() const;

	//****************************************************
	// static function
	//****************************************************

	// ゴーストの生成
	static CCollider* CreateGhost(const OBJ::Transform& Transform, Collision::SHAPETYPE Type = Collision::SHAPETYPE::BOX, float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);

private:

	//****************************************************
	// function
	//****************************************************
	void ErrorCheck(); // エラー検出

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<Impl> m_upImpl;
};