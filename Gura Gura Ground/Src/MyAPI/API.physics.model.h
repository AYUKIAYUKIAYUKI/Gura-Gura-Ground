//============================================================================
// 
// 物理モデル、ヘッダーファイル [physics.model.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.physics.object.h"
#include "API.model.gltf.h"

//****************************************************
// 物理モデルクラスの定義
//****************************************************
class CPhysicsModel : public CPhysicsObject
{
public:

	//****************************************************
	// special function
	//****************************************************

	// デフォルトコンストラクタ
	CPhysicsModel(OBJ::TYPE Type, OBJ::LAYER Layer);

	// デストラクタ
	~CPhysicsModel() override;

	//****************************************************
	// function
	//****************************************************

	// コライダーの生成
	virtual void FactoryCollider(float fWidth, float fHeight, float fDepth);

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// モデルのバインド
	inline void SetModel(GltfMesh* pData) { m_upModel->SetModel(pData); }

	// モデルオフセットの操作用
	inline const DirectX::XMFLOAT3& GetModelOffset() const                           { return m_ModelOffset; }
	inline       void               SetModelOffset(const DirectX::XMFLOAT3& rOffset) { m_ModelOffset = rOffset; }

private:

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<CGltf> m_upModel;     // glTFモデル
	DirectX::XMFLOAT3      m_ModelOffset; // モデルオフセット
};