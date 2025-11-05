
// このクラスは削除予定のため使用禁止です

//============================================================================
// 
// 任意のジオメトリ、ヘッダーファイル [any.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.object.h"

/* 仮 */
#include <GeometricPrimitive.h>
#include <Effects.h>
#include "API.world.h"

//****************************************************
// 任意のジオメトリクラスの定義
//****************************************************
class CAny : public CObject
{
public:

	//****************************************************
	// special function
	//****************************************************
	CAny(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CAny() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	//****************************************************
	// inline function
	//****************************************************

	// ジオメトリのバインド
	inline void SetGeometry(std::unique_ptr<DirectX::GeometricPrimitive>&& upGP) { m_upAnyGeometry = std::move(upGP); }

	// トランスフォーム全体の操作用
	inline const OBJ::Transform& GetTransform() const                          { return m_Transform; }
	inline void                  SetTransform(const OBJ::Transform& Transform) { m_Transform = Transform; }

	// サイズの操作用
	inline const DirectX::XMFLOAT3& GetSize() const                        { return m_Transform.Size; }
	inline void                     SetSize(const DirectX::XMFLOAT3& Size) { m_Transform.Size = Size; }

	// 向きの操作用
	inline const DirectX::XMFLOAT4& GetRot() const                       { return m_Transform.Rot; }
	inline void                     SetRot(const DirectX::XMFLOAT4& Rot) { m_Transform.Rot = Rot; }

	// 位置の操作用
	inline const DirectX::XMFLOAT3& GetPos() const                       { return m_Transform.Pos; }
	inline void                     SetPos(const DirectX::XMFLOAT3& Pos) { m_Transform.Pos = Pos; }

	// メッシュカラーの操作用
	inline const DirectX::XMFLOAT4& GetCol() const                       { return m_Col; }
	inline void                     SetCol(const DirectX::XMFLOAT4& Col) { m_Col = Col; }

	/* 検討 */
	inline void  SetCollisionShape(std::unique_ptr<btCollisionShape>&& upCS)  { m_upCollisionShape = std::move(upCS); }
	inline void  SetMotionState(std::unique_ptr<btDefaultMotionState>&& upMS) { m_upMotionState = std::move(upMS); }
	inline void  SetRigidBody(std::unique_ptr<btRigidBody>&& upRB)            { m_upRigidBody = std::move(upRB); }
	inline const std::unique_ptr<btCollisionShape>&     GetCollisionShape()   { return m_upCollisionShape; }
	inline const std::unique_ptr<btDefaultMotionState>& GetMotionState()      { return m_upMotionState; }
	inline const std::unique_ptr<btRigidBody>&          GetRigidBody()        { return m_upRigidBody; }

private:

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<DirectX::GeometricPrimitive> m_upAnyGeometry; // 任意のジオメトリ
	std::unique_ptr<DirectX::BasicEffect>        m_upEffect;      // 任意のエフェクト
	OBJ::Transform                               m_Transform;     // トランスフォーム
	DirectX::XMFLOAT4                            m_Col;           // メッシュカラー

	/* 検討 */
	std::unique_ptr<btCollisionShape>     m_upCollisionShape;
	std::unique_ptr<btDefaultMotionState> m_upMotionState;
	std::unique_ptr<btRigidBody>          m_upRigidBody;
};