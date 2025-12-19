//============================================================================
// 
// カメラの移動制御 [cameracontroller.h]
// Author : 後藤優輝
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.singleton.h"

//****************************************************
// 前方宣言
//****************************************************
class CCamera;
class CPlayer;
class CObstacle;

//****************************************************
// カメラの移動制御クラスの定義
//****************************************************
class CCameraController : public CSingleton<CCameraController>
{
	//****************************************************
	// フレンド宣言
	//****************************************************
	friend struct std::default_delete<CCameraController>;
	friend CCameraController& CSingleton<CCameraController>::RefInstance();

public:

	//****************************************************
	// function
	//****************************************************

	// 初期化処理
	bool Initialize();

	// 終了処理
	void Finalize();

	// 更新処理
	void Update();

	// プレイヤーの登録
	//void Regist(CPlayer* player);

	// プレイヤーの削除
	//void UnRegist(CPlayer* player);
private:

	//****************************************************
	// special function
	//****************************************************
	CCameraController(); // デフォルトコンストラクタ
	~CCameraController();// デストラクタ	

	//****************************************************
	// function
	//****************************************************

	// 中心位置を計算
	void CalculateCenter();
	
	// プレイヤーと定点のギミックから最小最大位置取得
	void GetPlayersAndObstaclesBounds(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max);

	// ギミックを取得
	void GetObstacles();

	// ギミックがあるか判定
	void HasMovingGimmick();

	// 外周移動ギミック
	void HasPerimeterGimmick();

	//****************************************************
	// data
	//****************************************************
	CCamera* m_Camera;						// カメラの情報
	//std::list<CPlayer*> m_Players;			// プレイヤーを格納
	std::list<CObstacle*> m_Obstacles;		// ギミックを格納
	DirectX::XMFLOAT3 m_CameraTargetPos;	// カメラの移動位置
	DirectX::XMFLOAT3 m_FirstCameraPos;		// カメラの最初の位置
	float m_BaseCameraDistance;				// カメラ基本の距離
	float m_MaxCameraDistance;				// カメラの最大の距離
	bool m_IsMovingGimmickActive;			// 特定方向に移動するギミックが出現しているか
	bool m_IsPerimeterGimmickActive;		// 外周を移動するギミックが出現しているか
};