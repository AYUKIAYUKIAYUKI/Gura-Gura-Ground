
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// glTFモデル、ヘッダーファイル [model.gltf.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.object.h"
#include "API.rigidbody.h"

//****************************************************
// 前方宣言
//****************************************************
struct GltfMesh;

//****************************************************
// glTFモデルクラスの定義
//****************************************************
class CGltf : public CObject
{
	//****************************************************
	// 前方宣言
	//****************************************************
	class Impl;

public:

	//****************************************************
	// special function
	//****************************************************

	// デフォルトコンストラクタ
	CGltf(OBJ::TYPE Type, OBJ::LAYER Layer);

	// デストラクタ
	~CGltf() override;

	//****************************************************
	// function
	//****************************************************

	// ファクトリ
	virtual void Factory();

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// モデルのバインド
	void SetModel(const GltfMesh& rData);

	// 頂点シェーダーのバインド
	void SetVertexShader(const ComPtr<ID3D11VertexShader>& rcpVS);

	// 入力レイアウトのバインド
	void SetInputLayout(const ComPtr<ID3D11InputLayout>& rcpIL);

	// ピクセルシェーダーのバインド
	void SetPixelShader(const ComPtr<ID3D11PixelShader>& rcpPS);

	// 定数バッファのバインド
	void SetConstantBuffer(const ComPtr<ID3D11Buffer>& rcpCB);

	// トランスフォームの操作用
	const OBJ::Transform& GetTransform() const;
	      void            SetTransform(const OBJ::Transform& TF);

	// リジッドボディの参照
	const RigidBody&                  RefRgidBody()     const;
	      std::unique_ptr<RigidBody>& UptrRefRgidBody() const;

private:

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<Impl> m_upImpl;
};