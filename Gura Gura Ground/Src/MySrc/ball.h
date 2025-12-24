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
#include "obstacle_editer.h"

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
	void FactoryCollider(float fWidth, float fHeight, float fDepth) override;

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// インスペクターの表示
	void ShowInspector() override;

	// パラメータの編集
	void EditParam() override;

	// 進行方向の設定
	inline const DirectX::XMFLOAT3& GetDirection() const                             { return m_Direction; }
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
};