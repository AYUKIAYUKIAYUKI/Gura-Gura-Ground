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
#include <memory>
#include <random>

//===================================================
//独自判断インクルード
#include <DirectXCollision.h>   // 当たり判定専用ヘッダー(BoundingOrientedBoxの使用)

//===================================================
//前方宣言
class CPlayer;
class CShockWave;
class CBar;


//===================================================
//敵プレイヤーのクラス ＝＝＝＝関数や変数が多いか,,,＝＝＝＝
class CEnemyPlayer :public CPhysicsModel
{
private: //構造体

	// オブジェクトの情報を管理する構造体（例）
	struct GameObject
	{
		//XMMATRIX worldMatrix;                //ワールド行列（回転・平行移動を含む）
		DirectX::BoundingOrientedBox localOBB; //ローカル座標系での初期ボックス
	};

	//比較に必要な情報群
	struct TargetInfo
	{
		float             distance;   //距離
		float             angle;      //向き
	    useful::Vec3      pos;        //位置
	    btVector3         vel;        //加速
	};

	//AIの応用パラメータ
	struct AIParams
	{
		int jumpcount = 0;
		float predictionTime;         //先読み時間
		float noiseangle;
		float weightDistance = 2.0f;  //距離を最重要に
		float weightApproach = 1.5f;  //接近度は強め
	};

	//各状態のタイプ
	enum class ENEMY_STATE
	{
		STATE_BASE,
		STATE_IN_JUMP,
		STATE_BAR,
	};

public: //自身に関する関数群

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

	/**
	 * @brief 別の自身クラスを探す処理
	 * @param [in] 自身のポインター
	 */
	void searchEnemy(std::shared_ptr<CEnemyPlayer> pSelf);

private: //プレイヤーに関する関数群
	/**
	 * @brief 敵をプレイヤーの方へ移動する関数
	 * @param [in] 角度、速度
	 */
	void MoveAtPlayer(float fAngle, float speed);

	/**
	 * @brief プレイヤーを探す処理
	 */
	void searchPlayer();

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
	 * @param [in] ジャンプフラグ、リキャストタイム、リキャストタイムの最大値
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

	/**
	 * @brief 比較処理(当たった時の判定や初動動かない処理)
	 * @param [in] 対象の位置,自身の位置,角度
	 */
	void Comparison(const DirectX::XMFLOAT3 targetPos, const DirectX::XMFLOAT3 SelfPos,float angle);

private: //その他

	/**
	 * @brief 落下判定中の処理(同じ処理を二回実行するから作成)
	 * @param [in] ジャンプフラグ、リキャストタイム、リキャストタイムの最大値
	 */
	bool DownHit(bool& bJump, int& RecastTme, const int MaxRecast);

	/**
	 * @brief 衝撃波の作成
	 * [in] 形の種類、大きさ、消えるまでの時間？
	 */
	void CreateShockWave(Collision::SHAPETYPE Type, const DirectX::XMFLOAT3A& Size, int nDuration);

	/**
	 * @brief 乱数
	 * [in] 最小値、最大値
	 */
	float RandomRange(float min, float max)
	{
		static std::mt19937 mt{ std::random_device{}() };
		std::uniform_real_distribution<float> dist(min, max);
		return dist(mt);
	}

	/**
	 * @brief min～maxの間の数値を乱数で渡し１/２で+-が変わる
	 * [in] 最小値、最大値
	 */
	double RandomSplit(float min, float max)
	{
		static std::mt19937 mt(std::random_device{}());

		// 0 or 1 をランダムに選ぶ
		std::uniform_int_distribution<int> choose(0, 1);

		// min～max の乱数
		std::uniform_real_distribution<double> dist(min, max);

		double v = dist(mt);

		// 50% の確率で負にする
		if (choose(mt) == 1)
			v = -v;

		return v;
	}


	/**
	 * @brief 正規化する処理
	 * @param [in] 正規化したい位置情報
	 */
	DirectX::XMFLOAT3 NormalizeFloat3(const DirectX::XMFLOAT3& v);

	/**
	 * @brief TargetInfo変数に情報を入れる処理
	 * [in] weekポインタ情報、TargetInfoの情報,自身の位置
	 */
	template <class T>
	void CollectTargetInfo(
		const std::vector<std::weak_ptr<T>>& sources,
		std::vector<TargetInfo>& out,
		const DirectX::XMFLOAT3& selfPos)
	{
		for (const auto& wp : sources)
		{
			if (auto sp = wp.lock())
			{
				auto pos = sp->GetTransform().Pos;

				CRigidBody* rb = DownCast<CRigidBody>(sp->GetCollider());
				auto vel = rb->GetLinearVelocity();

				//代入
				out.push_back
				({
					CheckDistance(pos, selfPos),
					atan2f(pos.x - selfPos.x, pos.z - selfPos.z),
					pos,
					vel
				});
			}
		}
	}

	/**
	 * @brief TargetInfo変数に情報を入れる処理
	 * [in] TargetInfoの情報、自身の位置
	 */
	float ScoreTarget(const TargetInfo& t, const DirectX::XMFLOAT3& selfPos);

	// 状態ごとの処理関数
	void State_Base();        //基礎状態
	void State_In_Jump();     //飛んだ後(最中)の処理
	void State_Bar();         //バーに関する状態

	//関数分け
	void State_Base_Search(); //敵（自身）とプレイヤーのベース（当たった時など）
	void State_Base_Bar();    //バーベース

private:

	//===================================================
	//プレイヤー参照変数
	CShockWave* m_pShockWave;                         // 衝撃波
	bool m_bGoDown;                                   //下降判定
	btVector3 m_btOldVel;                             //過去の加速値

    std::vector<std::weak_ptr<CPlayer>>m_pwPlayer;    //プレイヤーの閲覧用ポインター（複数人必要な為、vectorで管理）
    std::vector<std::weak_ptr<CEnemyPlayer>>m_pwSelf; //敵プレイヤーの閲覧用ポインター（複数人必要な為、vectorで管理）
	CBar* m_pBar;                                     //バーの情報を取得する用
									                
	AIParams      m_params;                           //AIの基本パラメータ
	int m_nStart;                                     //ゲーム開始の移動までのカウントを進める用
	bool m_bStart;                                    //ゲーム開始時移動していいかどうか判断用

	//===================================================
	//共通
	int m_nRecasttime;                                //行動までのリキャストタイム
	bool m_bJump;                                     //ジャンプするかどうかの判定用(true=ジャンプ可能)
	ENEMY_STATE m_State;                              //状態管理用変数
};