//======================================================
//
//ステイトパターン実装[state.h] 
//Auther:haruki Chiba
//
//======================================================


//==========================================================
//インクルードガード
#pragma once


//==========================================================
//インクルード
#include "player.h"

//==========================================================
//前方宣言
template<typename OwnerType>
class StateMachine;


//==========================================================
// 全ての状態の基底となるクラス
//==========================================================
template<typename OwnerType> //テンプレート引数で状態を管理する対象の型を指定
class CStateBase
{
public:
	/**
	* @brief 現在のステートを教えるよ
	*/
	virtual const char* GetStateName() = 0;

	/**
	* @brief 状態管理クラスの情報を返す処理
	*/
	inline StateMachine<OwnerType>* GetMacine() { return m_pMachine; }

protected:
	/**
	* @brief ステートマシン以外から呼び出し関数などにアクセスできないように
	*/
	friend class StateMachine<OwnerType>;

	/**
	* @brief  状態が開始したときに一度だけ呼ばれる処理
	* @param [in] (テンプレート) a_pOwner : ステートの持ち主を引数から取得できるように
	*/
	virtual void OnStart(OwnerType* a_pOwner) = 0;

	/**
	* @brief  状態が更新するときに呼ばれる処理
	* @param [in] (テンプレート) a_pOwner : ステートの持ち主を引数から取得できるように
	*/
	virtual void OnUpdate(OwnerType* a_pOwner) = 0;

	/**
	* @brief  状態が終了したときに一度だけ呼ばれる処理
	* @param [in] (テンプレート) a_pOwner : ステートの持ち主を引数から取得できるように
	*/
	virtual void OnExit(OwnerType* a_pOwner) = 0;

private:

	/**
	* @brief この状態を管理しているステートマシーンをセット
	* @param [in] (テンプレート) a_pOwner : ステートの持ち主を引数から取得できるように
	*/
	void SetMachine(StateMachine<OwnerType>* a_pMachine)
	{
		m_pMachine = a_pMachine;
	}

	/**
	* @brief  開始関数をマシンから呼ぶための関数
	* @param [in] (テンプレート) a_pOwner : ステートの持ち主を引数から取得できるように
	*/
	void CallStart(OwnerType* a_pOwner)
	{
		if (m_pMachine == nullptr || a_pOwner == nullptr)
		{
			return;
		}
		OnStart(a_pOwner);
	}

	/**
	* @brief  更新関数をマシンから呼ぶための関数
	* @param [in] (テンプレート) a_pOwner : ステートの持ち主を引数から取得できるように
	*/
	void CallUpdate(OwnerType* a_pOwner)
	{
		if (m_pMachine == nullptr || a_pOwner == nullptr)
		{
			return;
		}
		OnUpdate(a_pOwner);
	}

	/**
	* @brief  終了関数をマシンから呼ぶための関数
	* @param [in] (テンプレート) a_pOwner : ステートの持ち主を引数から取得できるように
	*/
	void CallExit(OwnerType* a_pOwner)
	{
		if (m_pMachine == nullptr || a_pOwner == nullptr)
		{
			return;
		}
		OnExit(a_pOwner);
	}

private:

	StateMachine<OwnerType>* m_pMachine = nullptr; // このステートを管理しているステートマシンのポインタを保存
};


//==========================================================
// 状態を管理するクラス
//==========================================================
template<typename OwnerType>
class StateMachine
{
public:
	/**
	* @brief コンストラクタ
	*/
	StateMachine() :m_fnChangeState([]() {}) {}

	/**
	* @brief デストラクタ
	*/
	~StateMachine()
	{
		ClearState();               //削除
		m_fnChangeState = []() {};  //念のため初期化
		m_lStates.clear();          //念のため削除
	}

	/**
	* @brief ステートマシンの開始関数
	*/
	void Start(OwnerType* a_pOwner)
	{
		m_pOwner = a_pOwner;
		m_fnChangeState = []() {};
	}

	/**
	* @brief 状態を変更する関数、 ステートの変更処理は更新の直前に呼ぶ
	* @param [in] ArgType...a_args : 引き数はステートのコンストラクタに渡す値
	*/
	template<typename StateType, typename...ArgType>
	void ChangeState(ArgType...a_args)
	{
		StateChanger<StateType>(true, a_args...);
	}

	/**
	* @brief 状態を追加する関数、 ステートの変更処理は更新の直前に呼ぶ
	* @param [in] ArgType...a_args : 引き数はステートのコンストラクタに渡す値
	*/
	template<typename StateType, typename...ArgType> // ステートを追加する関数を追加
	void PushState(ArgType...a_args)
	{
		StateChanger<StateType>(false, a_args...);
	}

	/**
	* @brief 今のステートをポップする
	*/
	void PopState()
	{
		m_fnChangeState = [&]()
		{
			// 一番後ろのステートを取得
			// 無ければreturn
			std::shared_ptr<CStateBase<OwnerType>> spNowState = GetNowState();
			if (spNowState == nullptr)
			{
				return;
			}

			// 終了関数を呼び、ポップする
			spNowState->CallExit(m_pOwner);
			m_lStates.pop_back();
		};
	}

	/**
	* @brief 今のステートを終了し、リストにあるステートをすべてクリアする
	*/
	void ClearState()
	{

		m_fnChangeState = [&]()
		{
			std::shared_ptr<CStateBase<OwnerType>> spNowState = GetNowState();
			if (spNowState == nullptr)
			{
				return;
			}
			spNowState->CallExit(m_pOwner);
			m_lStates.clear();
		};
	}


	// 状態の更新
	void Update()
	{
		// ステートの変更命令があれば処理する
		m_fnChangeState();
		m_fnChangeState = []() {};

		std::shared_ptr<CStateBase<OwnerType>> spBackState = GetNowState();

		if (spBackState != nullptr)
		{
			spBackState->CallUpdate(m_pOwner);
		}

	}

	// 現在有効なステートを取得
	std::shared_ptr<CStateBase<OwnerType>> GetNowState() // 変更点(3)
	{
		if (m_lStates.empty())
		{
			return nullptr;
		}
		std::shared_ptr<CStateBase<OwnerType>> spBackState = m_lStates.back();
		if (spBackState == nullptr)
		{
			return nullptr;
		}

		return spBackState;
	}

	///**
	// * @brief 現在のステートを返すよ
	// */
	//inline CStateBase<OwnerType>* GetState() { return m_spNowState.get(); };

	/**
	 * @brief 現在のステートを返すよ
	 */
	inline std::shared_ptr<CStateBase<OwnerType>>& GetState()
	{
		//listで管理しつつ加算していくため、常に追加された物を参照していく(一応popで削除済)
		return m_lStates.back();
	};

private:

	template<typename StateType, typename...ArgType>
	void StateChanger(bool a_isPop, ArgType...a_args)
	{
		// ステートの変更命令を格納する
		m_fnChangeState = [&]()
		{
			// オーナーがセットされてなければ何もしない
			if (m_pOwner == nullptr)
			{
				return;
			}

			std::shared_ptr<CStateBase<OwnerType>> spBackState = GetNowState();

			// もしすでにステートがセットされてたら終了する
			if (spBackState != nullptr)
			{
				spBackState->CallExit(m_pOwner);

				if (a_isPop)
				{

				}
				m_lStates.pop_back(); //現在のステートを削除
			}

			// 新しいステートを作成
			std::shared_ptr<CStateBase<OwnerType>> spNewState = std::make_shared<StateType>(a_args...);
			if (spNewState == nullptr)
			{
				return;
			}

			// 作成したステートをリストに追加
			m_lStates.emplace_back(spNewState);

			auto A = m_lStates;
			// 新しいステートにこのマシーンをセット
			spNewState->SetMachine(this);

			// ステートの開始
			spNewState->CallStart(m_pOwner);

		};
	}


private:

	// 状態の持ち主のポインタ
	OwnerType* m_pOwner = nullptr;

	// 今のステート
	std::list<std::shared_ptr<CStateBase<OwnerType>>> m_lStates;   //リストで保管

	// ステートの変更命令を保存しておく関数オブジェクト
	std::function<void()> m_fnChangeState;
};


//==========================================================
// プレイヤー専用の状態の基底クラス
//==========================================================
class CPlayerStateBase :public CStateBase<CPlayer>
{
public:
	/**
	 * @brief コンストラクタ
	 */
	CPlayerStateBase() {};

	/**
	 * @brief デストラクタ
	 */
	~CPlayerStateBase()
	{

	}

	/**
	 * @brief ジャンプ処理を呼ぶ処理
	 */
	bool CallJump(CPlayer* pPlayer);
};


//==========================================================
// プレイヤーの待機状態
//==========================================================
class CPlayer_DefaultState :public CPlayerStateBase
{
public:
	/**
	 * @brief コンストラクタ
	 */
	CPlayer_DefaultState() :m_bOneFlag(false)
	{};

	/**
	 * @brief デストラクタ
	 */
	~CPlayer_DefaultState() {};

	/**
	 * @brief 一度だけ呼ばれる初めの処理
	 * @param  オーバライドしている
	 */
	void OnStart(CPlayer* pPlayer)override;

	/**
	 * @brief 更新処理
	 * @param  オーバライドしている
	 */
	void OnUpdate(CPlayer* pPlayer)override;

	/**
	 * @brief 一度だけ呼ばれる終わりの処理
	 * @param  オーバライドしている
	 */
	void OnExit(CPlayer* pPlayer)override;

	/**
	* @brief 現在のステートを教えるよ
	*/
	const char* GetStateName()override;

private:
	bool m_bOneFlag; //一回だけ通す為の変数
};


//==========================================================
// プレイヤーの移動状態
//==========================================================
class CPlayer_MoveState :public CPlayerStateBase
{
public:
	/**
	 * @brief コンストラクタ
	 */
	CPlayer_MoveState() {};

	/**
	 * @brief デストラクタ
	 */
	~CPlayer_MoveState() {};

	/**
	 * @brief 一度だけ呼ばれる初めの処理
	 * @param  オーバライドしている
	 */
	void OnStart(CPlayer* pPlayer)override;

	/**
	 * @brief 更新処理
	 * @param  オーバライドしている
	 */
	void OnUpdate(CPlayer* pPlayer)override;

	/**
	 * @brief 一度だけ呼ばれる終わりの処理
	 * @param  オーバライドしている
	 */
	void OnExit(CPlayer* pPlayer)override;

	/**
	* @brief 現在のステートを教えるよ
	*/
	const char* GetStateName()override;
};


//==========================================================
// プレイヤーのお尻状態
//==========================================================
class CPlayer_OsiriState :public CPlayerStateBase
{
public:
	/**
	 * @brief コンストラクタ
	 */
	CPlayer_OsiriState() {};

	/**
	 * @brief デストラクタ
	 */
	~CPlayer_OsiriState() {};

	/**
	 * @brief 一度だけ呼ばれる初めの処理
	 * @param  オーバライドしている
	 */
	void OnStart(CPlayer* pPlayer)override;

	/**
	 * @brief 更新処理
	 * @param  オーバライドしている
	 */
	void OnUpdate(CPlayer* pPlayer)override;

	/**
	 * @brief 一度だけ呼ばれる終わりの処理
	 * @param  オーバライドしている
	 */
	void OnExit(CPlayer* pPlayer)override;

	/**
	* @brief 現在のステートを教えるよ
	*/
	const char* GetStateName()override;
};


//==========================================================
// プレイヤーのジャンプ状態
//==========================================================
class CPlayer_JumpState :public CPlayerStateBase
{
public:
	/**
	 * @brief コンストラクタ
	 */
	CPlayer_JumpState() {};

	/**
	 * @brief デストラクタ
	 */
	~CPlayer_JumpState() {};

	/**
		 * @brief 一度だけ呼ばれる初めの処理
		 * @param  オーバライドしている
		 */
	void OnStart(CPlayer* pPlayer)override;

	/**
	 * @brief 更新処理
	 * @param  オーバライドしている
	 */
	void OnUpdate(CPlayer* pPlayer)override;

	/**
	 * @brief 一度だけ呼ばれる終わりの処理
	 * @param  オーバライドしている
	 */
	void OnExit(CPlayer* pPlayer)override;

	/**
	* @brief 現在のステートを教えるよ
	*/
	const char* GetStateName()override;
};


//==========================================================
// プレイヤーのダメージ状態
//==========================================================
class CPlayer_DamageState :public CPlayerStateBase
{
public:
	/**
	 * @brief コンストラクタ
	 */
	CPlayer_DamageState() {};

	/**
	 * @brief デストラクタ
	 */
	~CPlayer_DamageState() {};

	/**
	 * @brief 一度だけ呼ばれる初めの処理
	 * @param  オーバライドしている
	 */
	void OnStart(CPlayer* pPlayer)override;

	/**
	 * @brief 更新処理
	 * @param  オーバライドしている
	 */
	void OnUpdate(CPlayer* pPlayer)override;

	/**
	 * @brief 一度だけ呼ばれる終わりの処理
	 * @param  オーバライドしている
	 */
	void OnExit(CPlayer* pPlayer)override;

	/**
	* @brief 現在のステートを教えるよ
	*/
	const char* GetStateName()override;

private:
	int nCount = 0;
};