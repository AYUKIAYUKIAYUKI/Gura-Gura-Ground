
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// 四角形(2D)、ヘッダーファイル [rect.2D.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.object.h"

//****************************************************
// 四角形(2D)クラスの定義
//****************************************************
class CRect2D : public CObject
{
	//****************************************************
	// 前方宣言
	//****************************************************
	class Impl;

public:

	//****************************************************
	// special function
	//****************************************************
	CRect2D(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CRect2D() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// 頂点シェーダーのバインド
	void SetVertexShader(const ComPtr<ID3D11VertexShader>& rcpVS);

	// 入力レイアウトのバインド
	void SetInputLayout(const ComPtr<ID3D11InputLayout>& rcpIL);

	// ピクセルシェーダーのバインド
	void SetPixelShader(const ComPtr<ID3D11PixelShader>& rcpPS);

	// テクスチャのバインド
	void SetTexture(const ComPtr<ID3D11ShaderResourceView>& rcpSRV);

	// WVP行列用定数バッファのバインド
	inline void SetConstantBuffer_WVP(const ComPtr<ID3D11Buffer>& rcpCB_WVP);

	// カラー用定数バッファのバインド
	void SetConstantBuffer_Col(const ComPtr<ID3D11Buffer>& rcpCB_Col);

	// トランスフォームの操作用
	const OBJ::Transform& GetTransform() const;
	      void            SetTransform(const OBJ::Transform& TF);

	// 色の操作用
	const DirectX::XMFLOAT4& GetCol() const;
		  void               SetCol(const DirectX::XMFLOAT4& Col);

private:

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<Impl> m_upImpl;
};