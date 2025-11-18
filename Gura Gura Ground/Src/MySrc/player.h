//============================================================================
// 
// プレイヤー、ヘッダーファイル [player.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.model.gltf.h"

template<typename T>
class StateMachine;

//****************************************************
// プレイヤークラスの定義
//****************************************************
class CPlayer : public CGltf
{
	//****************************************************
	// 静的メンバ定数を定義 (非公開)
	//****************************************************

	/* 最終的には定数となりますが、漸次的に編集可能な値とします */

	// 目標値への補間係数
	static float COEF_CORRECT_TARGET;

	// 重力加速度
	static float COEF_GRAVITY;

	// ジャンプ可能回数
	static int NUM_LEFT_JUMP;

	// ジャンプ力
	static float COEF_TRIGGER_JUMP;

	// 移動速度
	static float COEF_MOVE_SPEED_AIR;

	// 制動力
	static float COEF_BRAKING;

	// 半径
	static float RADIUS;


public:

	//****************************************************
	// 静的メンバ変数の宣言 (公開)
	//****************************************************
	static float COEF_MOVE_SPEED; //移動値

	// デフォルトのファクトリ
	static std::function<bool(CPlayer*)> s_fpDefaultFactory;

	//****************************************************
	// special function
	//****************************************************
	CPlayer(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
	~CPlayer() override;                       // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// ファクトリ
	void Factory(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f) override;

	// 初期化処理
	bool Initialize();

	// 終了処理
	void Finalize();

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	//****************************************************
	// inline function
	//****************************************************

	// 残りジャンプ回数操作用
	//inline int  GetLeftNumJump() const   { return m_nLeftNumJump; }
	//inline void SetLeftNumJump(int nNum) { m_nLeftNumJump = nNum; }

	// 加速度操作用
	inline const DirectX::XMFLOAT3& GetVelocity() const { return m_Velocity; }
	inline void						 SetVelocity(const DirectX::XMFLOAT3& Vel) { m_Velocity = Vel; }

	// 目標サイズ操作用
	inline const DirectX::XMFLOAT3& GetSizeTarget() const { return m_SizeTarget; }
	inline void						SetSizeTarget(const DirectX::XMFLOAT3& SizeTarget) { m_SizeTarget = SizeTarget; }

	//**//==========================================================================================================

	/**
	 * @brief 移動処理
	 * @param [in] fSpeed : 移動値
	 */
	void Move(float fSpeed);

	/**
	 * @brief 入力されたかどうか判定する処理
	 */
	bool JudgeInput();

	/**
	 * @brief 飛ぶ処理
	 */
	void Jump();

	/**
	 * @brief 飛んでいる時の処理
	 */
	bool InJump();

	/**
	 * @brief ダメージ処理
	 */
	void Damage();

	bool Hit();				 // 攻撃当たった処理

	float GetRadius();		//半径を返す

	/**
	 * @brief 何もない処理
	 */
	inline void Null() {};

	/**
	 * @brief ステイトマシーンの情報を返す処理
	 */
	StateMachine<CPlayer>* Getmachine() { return m_stateMachine; }

private:

		//****************************************************
		// function
		//****************************************************

		// 操作など
		bool Gravity();          // 重力加速
		void SetWave();          // 振動設定
		void PlayWave();         // 振動再生
		void ValueEdit();        // 数値編集
		void ExportStatus();//ステータスを書き出す
		const char* ToString();//現在のステートを返す処理

		//****************************************************
		// data
		//****************************************************
		int         m_nFrame;       // フレーム数
		//int       m_nLeftNumJump; // ジャンプ可能回数
		DirectX::XMFLOAT3 m_Velocity;     // 加速度
		DirectX::XMFLOAT3 m_SizeTarget;   // 目標サイズ

		// 振動再生用：目標サイズリスト
		using Michos = std::list<DirectX::XMFLOAT3>;
		Michos m_vMichos;

		// ステートマシン
		StateMachine<CPlayer>* m_stateMachine;

};