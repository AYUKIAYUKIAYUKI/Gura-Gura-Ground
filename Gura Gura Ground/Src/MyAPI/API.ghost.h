
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
class btPairCachingGhostObject;

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
	CGhost(Collision::SHAPETYPE Type, float fWidth, float fHeight, float fDepth);

	// デストラクタ
	~CGhost();

	//****************************************************
	// function
	//****************************************************

	// 更新処理
	void Update(const OBJ::Transform& TF) override;

	// 描画処理
	void Draw() override;

	// ゴースト本体のユニークポインタの参照
	//      std::unique_ptr<btPairCachingGhostObject>& UptrRefGhost();
	//const std::unique_ptr<btPairCachingGhostObject>& UptrRefGhostConst() const;

	// ゴーストの生成
	static void CreateGhost(std::unique_ptr<CGhost>&    upGhost, Collision::SHAPETYPE Type = Collision::SHAPETYPE::BOX, float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);
	static void CreateGhost(std::unique_ptr<CCollider>& upCL,    Collision::SHAPETYPE Type = Collision::SHAPETYPE::BOX, float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);

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