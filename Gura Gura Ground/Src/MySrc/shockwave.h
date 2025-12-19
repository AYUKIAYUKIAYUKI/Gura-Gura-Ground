//============================================================================
// 
// 衝撃波、ヘッダーファイル [shockwave.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.physics.object.h"

//****************************************************
// 衝撃波クラスの定義
//****************************************************
class CShockWave : public CPhysicsObject
{
public:

	//****************************************************
	// special function
	//****************************************************
	CShockWave(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CShockWave() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// コライダーのファクトリ
	void FactoryCollider(Collision::SHAPETYPE Type, float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// 無視対象の設定
	inline void SetIgnore(const std::shared_ptr<CObject>& spIgnore) { m_wpIgnore = spIgnore; }

	// 期間の設定
	inline void SetDuration(int nDuration) { m_nDuration = nDuration; }

private:

	//****************************************************
	// function
	//****************************************************
	void Push(); // 押し出し処理

	//****************************************************
	// data
	//****************************************************
	std::weak_ptr<CObject> m_wpIgnore;  // 無視対象
	int                    m_nDuration; // 期間
};