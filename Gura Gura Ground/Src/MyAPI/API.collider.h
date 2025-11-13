
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// コライダー、ヘッダーファイル [collider.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************

// 名前空間OBJへのアクセス用
#include "API.object.h"

//****************************************************
// 前方宣言
//****************************************************
class  btCollisionShape;
struct RenderCollision;

//****************************************************
// 名前空間Collsion：衝突判定にまつわる定義を配置します
//****************************************************
namespace Collision
{
	//****************************************************
	// 形状指定の列挙型を定義
	//****************************************************
	enum class SHAPETYPE : unsigned char
	{
		BOX = 0,
		SPHERE,
		CYLINDER,
		CONE
	};
}

//****************************************************
// コライダークラスの定義
//****************************************************
class CCollider
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
	CCollider(Collision::SHAPETYPE Type, float fWidth, float fHeight, float fDepth);

	// デストラクタ
	virtual ~CCollider();

	//****************************************************
	// function
	//****************************************************

	// 更新処理
	virtual void Update(const OBJ::Transform& TF);

	// 描画処理
	virtual void Draw();

protected:

	//****************************************************
	// function：継承先からの設定変更のため
	//****************************************************

	// コリジョン形状の参照
	      btCollisionShape& RefCollisionShape();
	const btCollisionShape& RefCollisionShapeConst() const;

	// コリジョン形状のユニークポインタの参照
          std::unique_ptr<btCollisionShape>& UptrRefCollisionShape();
	const std::unique_ptr<btCollisionShape>& UptrRefCollisionShapeConst() const;

	// コリジョン描画のユニークポインタの参照
	      std::unique_ptr<RenderCollision>& UptrRefRenderCollision();
	const std::unique_ptr<RenderCollision>& UptrRefRenderCollisionConst() const;

	// コリジョン描画のカラー設定
	void SetRenderCollisionColor(const DirectX::XMFLOAT4& Color);

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