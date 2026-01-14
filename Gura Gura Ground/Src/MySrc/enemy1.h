//===================================================
//
//敵プレイヤーのクラス(仮)[enemy.h]
//プレイヤーの処理を参考
//Auther:Haruki Chiba
//
//===================================================


//===================================================
//インクルードガード
#pragma once

//===================================================
//s必要なインクルード
#include "API.physics.model.h" //基底クラス
#include <API.collision.h>      //btVector3の使用

//===================================================
//独自判断インクルード
#include <DirectXCollision.h>   // 当たり判定専用ヘッダー(BoundingOrientedBoxの使用)

//===================================================
//前方宣言
class CPlayer;
class CShockWave;
class CBar;


//===================================================
//敵プレイヤーのクラス
class CEnemyPlayer :public CPhysicsModel
{
private:
	// オブジェクトの情報を管理する構造体（例）
	struct GameObject
	{
		//XMMATRIX worldMatrix;         // ワールド行列（回転・平行移動を含む）
		DirectX::BoundingOrientedBox localOBB; // ローカル座標系での初期ボックス
	};

	//各状態のタイプ
	enum class ENEMY_STATE
	{
		STATE_BASE,
		STATE_IN_JUMP,
		STATE_BAR,
	};

	// 状態ごとの処理関数
	void State_Base();     //基礎状態
	void State_In_Jump();  //飛んだ後(最中)の処理
	void State_Bar();      //バーに関する状態

public:

	/**
	 * @briefコンストラクタ
	 */
	CEnemyPlayer(OBJ::TYPE Type, OBJ::LAYER Layer);

	/**
	 * @brief デストラクタ
	 */
	~CEnemyPlayer();

	// コライダーのファクトリ
	void FactoryCollider(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f) override;

	/**
	 * @brief 更新処理
	 */
	void Update() override;

	/**
	 * @brief 描画処理
	 */
	void Draw() override;

private: //プレイヤーに関する関数群

	/**
	 * @brief 敵をプレイヤーの方へ移動する関数
	 */
	void MoveAtPlayer(float fAngle, float speed);

	/**
	 * @brief プレイヤーを探す処理
	 */
	void searchPlayer();

	/**
	 * @brief 衝撃波の作成
	 */
	void CreateShockWave(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3A& Size, int nDuration);

	/**
	 * @brief 自身とプレイヤーの当たり判定チェック処理
	 * @param [in] 対象の位置情報、自身の位置情報,範囲
	 */
	bool CheckCollision(const DirectX::XMFLOAT3& c1, const DirectX::XMFLOAT3& c2, float Radius);

	/**
	 * @brief 距離を算出する処理
	 * @param [in] 対象の位置情報、自身の位置情報
	 */
	float CheckDistance(const DirectX::XMFLOAT3& c1, const DirectX::XMFLOAT3& c2);

private: //バーに関する関数群

	/**
	 * @brief バーの探す処理
	 */
	void searchBar();

	/**
	 * @brief Obb情報を設定する処理（値を設定するので参照渡し）
	 * @param [in] GameObject構造体の情報、位置、大きさ（FactoryCollider参照）、向き
	 */
	GameObject& SetObbInfo(GameObject& Obj, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 size, const DirectX::XMFLOAT4 rot);

private: //共通する関数群

	/**
	 * @brief 飛んでいる最中
	 */
	bool InJump(bool& bJump, int& RecastTme, const int MaxRecast);

	/**
	 * @brief ジャンプする時の条件をまとめた関数
	 */
	void Jump_Base();

	/**
	 * @brief 状態遷移関数
	 */
	void ChangeState(ENEMY_STATE next)
	{
		m_State = next;
	}

	/**
	 * @brief 情報確認
	 */
	void CheckInfo();

private:

	//===================================================
	//プレイヤー参照変数
	CShockWave* m_pShockWave;          // 衝撃波
	bool m_bGoDown;
	btVector3 m_btOldVel;

	std::vector<CPlayer*>m_pPlayer;  //プレイヤーの情報を取得する用
	CBar* m_pBar;                    //バーの情報を取得する用

	int m_nStart;                    //ゲーム開始の移動までのカウントを進める用
	bool m_bStart;                   //ゲーム開始時移動していいかどうか判断用

	//===================================================
	//共通
	int m_nRecasttime;               //行動までのリキャストタイム
	bool m_bJump;                    //ジャンプするかどうかの判定用(true=ジャンプ可能)
	ENEMY_STATE m_State;             //状態管理用変数
};