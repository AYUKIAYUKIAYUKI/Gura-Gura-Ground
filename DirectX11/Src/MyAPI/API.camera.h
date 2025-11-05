
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// カメラ、ヘッダーファイル [camera.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// カメラクラスの定義
//****************************************************
class alignas(16) CCamera
{
	//****************************************************
	// 静的メンバ定数の定義 (非公開)
	//****************************************************

	// ニア・ファークリップ
	static constexpr float CLIP_NEAR = 0.1f;
	static constexpr float CLIP_FAR  = 100.0f;

	// 補間速度
	static constexpr float COEF_CORRECT = 0.15f;

public:

	//****************************************************
	// special function
	//****************************************************
	CCamera();  // デフォルトコンストラクタ
	~CCamera(); // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// 更新処理
	void Update();

	//****************************************************
	// inline function
	//****************************************************

	// ビュー行列を参照
	inline const DirectX::XMMATRIX& GetView() const { return m_View; }

	// 投影行列を参照
	inline const DirectX::XMMATRIX& GetProjection() const { return m_Projection; }

	// ワールド行列を参照
	inline const DirectX::XMMATRIX& GetWorld() const { return m_World; }

	// WVP行列を参照
	inline const DirectX::XMMATRIX& GetWVP() const { return m_WVP; }

private:

	//****************************************************
	// function
	//****************************************************
	void ControlImGui();         // ImGuiでの操作
	void ControlFly();           // フライ操作
	void CameraTransformFly();	 // フライ操作でのカメラ適応
	void ControlOrbit();         // オービット操作
	void CameraTransformOrbit(); // オービット操作でのカメラ適応
	void CorrectToTarget();		 // 目標値への補間
	void AutoRestrictPitch();	 // ピッチ角の範囲制限

	//****************************************************
	// data
	//****************************************************
	bool              m_bUseFirstPerson;  // 一人称視点の使用状態
	bool              m_bUseControlDrag;  // ドラッグ操作の使用状態
	bool              Padding0;
	bool              Padding1;
	float             m_fDragMoveSpeed;   // ドラッグ操作時の移動速度
	float             m_fDragRotateSpeed; // ドラッグ操作時の回転速度
	float             Padding2;
	DirectX::XMFLOAT3 m_Rot;              // 向き
	DirectX::XMFLOAT3 m_RotTarget;        // 目標向き
	DirectX::XMFLOAT3 m_Pos;              // 位置
	DirectX::XMFLOAT3 m_PosTarget;        // 目標位置
	float		      m_fDistance;        // 距離
	float		      m_fDistanceTarget;  // 目標距離
	float		      m_fUpAdjust;        // 俯瞰度合い
	float		      m_fUpAdjustTarget;  // 目標俯瞰度合い
	DirectX::XMVECTOR m_CalcPos;          // 計算用位置
	DirectX::XMVECTOR m_Target;           // 注視点位置
	DirectX::XMVECTOR m_Up;               // 上方向ベクトル
	DirectX::XMMATRIX m_View;             // ビュー行列
	DirectX::XMMATRIX m_Projection;       // 投影行列
	DirectX::XMMATRIX m_World;            // ワールド行列
	DirectX::XMMATRIX m_WVP;              // WVP行列
};