//============================================================================
// 
// 振り子 [pendulum.h]
// Author : 大竹熙
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "obstacle.h"

//****************************************************
// バークラスの定義
//****************************************************
class CPendulum : public CObstacle
{
public:

	//****************************************************
	// special function
	//****************************************************
	CPendulum(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CPendulum() override;                       // デストラクタ

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
	inline const DirectX::XMFLOAT3& GetDirection() const { return m_Direction; }
	inline       void               SetDirection(const DirectX::XMFLOAT3& Direction) { m_Direction = Direction; }

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
	float m_Time;      // 出現からの経過時間
	float m_Phase = 0.0f;   // 揺れの位相オフセット
};