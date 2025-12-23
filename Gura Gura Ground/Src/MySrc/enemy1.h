//===================================================
//
//敵のクラス(仮)[enemy1.h]
//プレイヤーの処理を参考
//Auther:Haruki Chiba
//
//===================================================


//===================================================
//インクルードガード
#pragma once


//===================================================
//インクルード
#include "API.physics.object.h"
#include "API.world.h"

//===================================================
//前方宣言
class CPlayer;
class CShockWave;


//===================================================
//敵のクラス
class CEnemy1 :public CPhysicsObject
{
public:
	
	/**
	 * @briefコンストラクタ
	 */
	CEnemy1(OBJ::TYPE Type, OBJ::LAYER Layer);

	/**
	 * @brief デストラクタ
	 */
	~CEnemy1();

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

	/**
	 * @brief プレイヤーを探す処理
	 */
	void searchPlayer();

	/**
	 * @brief  プレイヤーに対する各情報を計算する処理（位置や向きなど）
	 */
	void Calculation();

	/**
	 * @brief 敵をプレイヤーの方へ移動する関数
	 */
	void MoveAtPlayer(float fAngle, float speed);

	/**
	 * @brief 行動処理
	 * @param [in] プレイヤーの情報(ポインター),向き
	 */
	void Action(CPlayer* pPlayer, float fAngle);

	/**
	 * @brief 行動時範囲内にいる時の処理関数
	 */
	void ActionInColi();

	/**
	 * @brief 飛ぶ処理
	 */
	void Jump();

	/**
	 * @brief 飛んでる処理
	 */
	void InJump();

	/**
	 * @brief ヒップドロップ処理
	 */
	void HipDrap();

	// 衝撃波の作成
	void CreateShockWave(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3A& Size, int nDuration);

	/**
	 * @brief プレイヤーの情報を消す処理
	 */
	void DeletePlayerInfo();

	/**
	 * @brief 自身を消す処理（プレイヤーと同じ条件）
	 */
	void DeleteSelf();

	/**
	 * @brief 敵とプレイヤーの当たり判定チェック処理
	 * @param [in] 対象の位置情報、自身の位置情報,範囲
	 */
	bool CheckCollision(const DirectX::XMFLOAT3& c1, const DirectX::XMFLOAT3& c2, float Radius);

	/**
	 * @brief 距離を算出する処理
	 * @param [in] 対象の位置情報、自身の位置情報
	 */
	float CheckDistance(const DirectX::XMFLOAT3& c1, const DirectX::XMFLOAT3& c2);

private:

	//プレイヤーから引継ぎ
	CShockWave* m_pShockWave; // 衝撃波

	bool m_bGoDown;           //下降中かどうかの判定
	btVector3 m_btOldVel;     //過去の加速度

	//===================================================
	//オリジナル要素
	std::vector<CPlayer*>m_pPlayer;  //プレイヤーの情報を取得する用
	int m_nRecasttime;               //行動までのリキャストタイム
	bool m_bJump;                    //ジャンプするかどうかの判定用(true=ジャンプ可能)


	//===================================================
	//マクロ定義
	static constexpr int MAX_RECASTTIME = 120;         //リキャストタイムの最大値
	static constexpr float MOVE = 3.0f;                //自身の移動値
	static constexpr float JUMPPOWER = 10.0f;          //自身のジャンプ力(落下速度も兼ねている)
	static constexpr float TOP_POS_Y = JUMPPOWER*1.1f; //自身のジャンプ時の頂点
};