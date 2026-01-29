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
	virtual void FactoryCollider(float fWidth, float fHeight, float fDepth, Collision::SHAPETYPE Type);

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// モデルの操作用
	inline void SetModel(GltfMesh* pData) { m_upModel->SetModel(pData); }

	// モデルのピクセルシェーダーを設定
	inline void SetPixelShader(const ComPtr<ID3D11PixelShader>& rcpPS) { m_upModel->SetPixelShader(rcpPS); }

	// モデルオフセットの操作用
	inline const DirectX::XMFLOAT3& GetModelOffset() const                           { return m_ModelOffset; }
	inline       void               SetModelOffset(const DirectX::XMFLOAT3& rOffset) { m_ModelOffset = rOffset; }

	/* 回転同期の解除 */
	inline void DisableSyncRotation() { m_bSyncRotation = false; }

	/* 回転の操作用 */
	inline const DirectX::XMFLOAT3& GetRotation() const                        { return m_Rotation; }
	inline       void               SetRotation(const DirectX::XMFLOAT3& rRot) { m_Rotation = rRot; }

private:

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<CGltf> m_upModel;     // glTFモデル
	DirectX::XMFLOAT3      m_ModelOffset; // モデルオフセット

	/* しぶしぶ */
	bool                   m_bSyncRotation; // 回転同期フラグ
	DirectX::XMFLOAT3      m_Rotation;      // 回転
};