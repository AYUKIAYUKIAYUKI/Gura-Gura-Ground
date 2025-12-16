//============================================================================
// 
// 竜巻、ヘッダーファイル [tornado.h]
// Author : 後藤優輝
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "obstacle.h"
#include "API.collider.h"

//****************************************************
// 竜巻クラスの定義
//****************************************************
class CTornado : public CObstacle
{
public:

	//****************************************************
	// special function
	//****************************************************
	CTornado(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CTornado() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// コライダーのファクトリ
	void FactoryCollider(float fWidth, float fHeight, float fDepth) override;

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// パラメータの編集
	void EditParam() override;

	// 最初の位置の設定
	inline void SetStartPos(DirectX::XMFLOAT3 StartPos) { m_StartPos = StartPos; }

	// 横幅の設定
	inline void SetWidth(float Width) { m_Width = Width; }

	// 縦幅の設定
	inline void SetDepth(float Depth) { m_Depth = Depth; }
private:

	//****************************************************
	// function
	//****************************************************
	void SetMoveDir();	// 移動方向を設定

	//****************************************************
	// data
	//****************************************************
	DirectX::XMFLOAT3 m_StartPos;	// 移動を開始する位置
	DirectX::XMFLOAT3 m_MoveDir;	// 移動方向
	int m_NowEdge;					// 現在の辺
	float m_edgeProgress;			// 現在の辺の進行度
	float m_Width;					// 横幅
	float m_Depth;					// 縦幅
};