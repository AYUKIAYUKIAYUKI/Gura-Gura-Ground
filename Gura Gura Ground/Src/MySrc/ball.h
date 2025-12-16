//============================================================================
// 
// ボール、ヘッダーファイル [ball.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "obstacle.h"

//****************************************************
// ボールクラスの定義
//****************************************************
class CBall : public CObstacle
{
public:

	//****************************************************
	// special function
	//****************************************************
	CBall(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CBall() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// コライダーのファクトリ
	void FactoryCollider(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f) override;

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// パラメータの編集
	void EditParam() override { int i = 0; }

	// 進行方向の設定
	inline const DirectX::XMFLOAT3& GetDirection() const                             { return m_Direction; }
	inline       void               SetDirection(const DirectX::XMFLOAT3& Direction) { m_Direction = Direction; }

	void SetParamSetIndex(int idx) { m_ParamSetIndex = idx; }
	void SetSubParamIndex(int idx) { m_SubParamIndex = idx; }
	int  GetParamSetIndex() const { return m_ParamSetIndex; }
	int  GetSubParamIndex() const { return m_SubParamIndex; }

private:

	//****************************************************
	// function
	//****************************************************
	void Appear(); // 出現
	void Action(); // 挙動
	void Loop();   // 戻る

	//****************************************************
	// data
	//****************************************************
	DirectX::XMFLOAT3 m_Direction; // 進行方向
	int m_ParamSetIndex = 0;   // どのParamSetか
	int m_SubParamIndex = 0;   // その中の何番目か
};