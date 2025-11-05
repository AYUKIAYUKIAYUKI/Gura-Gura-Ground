//============================================================================
// 
// フィールド、ヘッダーファイル [filed.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "any.h"

//****************************************************
// フィールドクラスの定義
//****************************************************
class CField : public CAny
{
public:

	//****************************************************
	// special function
	//****************************************************
	CField(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CField() override;                       // デストラクタ

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

	/* 検討 */
	inline void SetGeometryC(std::unique_ptr<DirectX::GeometricPrimitive>&& upGP) { m_upAnyGeometry = std::move(upGP); }

	/* 検討 */
	inline void  SetCollisionShapeC(std::unique_ptr<btCollisionShape>&& upCS)   { m_upCollisionShape = std::move(upCS); }
	inline void  SetMotionStateC(std::unique_ptr<btDefaultMotionState>&& upMS)  { m_upMotionState = std::move(upMS); }
	inline void  SetRigidBodyC(std::unique_ptr<btRigidBody>&& upRB)             { m_upRigidBody = std::move(upRB); }
	inline void  SetConstraintC(std::unique_ptr<btPoint2PointConstraint>&& upC) { m_upConstraint = std::move(upC); }
	inline const std::unique_ptr<btCollisionShape>& GetCollisionShapeC()        { return m_upCollisionShape; }
	inline const std::unique_ptr<btDefaultMotionState>& GetMotionStateC()       { return m_upMotionState; }
	inline const std::unique_ptr<btRigidBody>& GetRigidBodyC()                  { return m_upRigidBody; }
	inline const std::unique_ptr<btPoint2PointConstraint>& GetConstraintC()     { return m_upConstraint; }

private:

	//****************************************************
	// data
	//****************************************************

	/* 検討 */
	std::unique_ptr<DirectX::GeometricPrimitive> m_upAnyGeometry;
	OBJ::Transform                               m_Transform;
	std::unique_ptr<btCollisionShape>            m_upCollisionShape;
	std::unique_ptr<btDefaultMotionState>        m_upMotionState;
	std::unique_ptr<btRigidBody>                 m_upRigidBody;
	std::unique_ptr<btPoint2PointConstraint>     m_upConstraint;
};