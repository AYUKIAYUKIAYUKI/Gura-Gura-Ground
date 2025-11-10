
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
#include "API.object.h"

//****************************************************
// 剛体情報の構造体
//****************************************************
enum class SHAPETYPE : unsigned char
{
	NONE = 0,
	BOX,
	SPHERE,
	CYLINDER,
	CONE
};

//****************************************************
// 前方宣言
//****************************************************
struct btDefaultMotionState;
class  btRigidBody;

//****************************************************
// リジッドボディ構造体の定義
//****************************************************
class RigidBody
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
	// Type -> 形状のタイプですが、これに応じて引数の意味が変わります
	// Type::BOX      ⇒ fWidth：幅, fHeight：高さ, fDepth：奥行き 
	// Type::SPHERE   ⇒ fWidth：直径
	// Type::CYLINDER ⇒ fWidth：幅, fHeight：高さ, fDepth：奥行き
	// Type::CONE     ⇒ fWidth：直径, fHeight：高さ
	RigidBody(SHAPETYPE Type, float fWidth, float fHeight, float fDepth);

	// デストラクタ
	~RigidBody();

	//****************************************************
	// function
	//****************************************************

	// ワイヤーの更新
	// TF -> ワイヤーが付属する対象のトランスフォーム情報
	void UpdateWire(const OBJ::Transform& TF);

	// ワイヤーの描画
	void DrawWire();

	// モーションステートのユニークポインタを参照
	const std::unique_ptr<btDefaultMotionState>& UPtrRefMotionState() const;

	// リジッドボディのユニークポインタを参照
	const std::unique_ptr<btRigidBody>& UPtrRefRigidBody() const;

	// リジッドボディの生成
	static void CreateRigidBody(std::unique_ptr<RigidBody>& upBase, SHAPETYPE Type = SHAPETYPE::BOX, float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);

private:

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<Impl> m_upImpl;
};