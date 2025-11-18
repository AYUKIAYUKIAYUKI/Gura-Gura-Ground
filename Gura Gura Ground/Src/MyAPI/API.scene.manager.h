//============================================================================
// 
// シーンマネージャー、ヘッダーファイル [scene.manager.h]
// Author : 福田歩希
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
class CScene;

//****************************************************
// コンセプトの定義
//****************************************************
template <typename T>
concept DerivedFromScene = std::derived_from<T, CScene>
&& requires(T t)
{
	{ t.Update() } -> std::same_as<void>;
}
&& requires(T t)
{
	{ t.Change() } -> std::same_as<void>;
};

//****************************************************
// シーンマネージャークラスの定義
//****************************************************
class CSceneManager final : public CSingleton<CSceneManager>
{
	//****************************************************
	// フレンド宣言
	//****************************************************
	friend struct std::default_delete<CSceneManager>;
	friend CSceneManager& CSingleton<CSceneManager>::RefInstance();

public:

	//****************************************************
	// function
	//****************************************************
	
	// 更新処理
	void Update();

	// 初期化処理
	template <DerivedFromScene T>
	bool Initialize();

	// シーン変更
	template <DerivedFromScene T>
	void ChangeScene(useful::up<T>&& upScene);

	// シーントランジション
	template <typename T>
	void TransitionScene(T&& fpDelayBehavior);

	//****************************************************
	// inline function
	//****************************************************

	// シーンの参照
	inline CScene& RefScene() const { return *useful::PtrCheck(m_upScene.get(), "Scene"); }

private:

	//****************************************************
	// special function
	//****************************************************
	CSceneManager();           // デフォルトコンストラクタ
	~CSceneManager() override; // デストラクタ

	//****************************************************
	// function
	//****************************************************
	
	// 継承用ダミー
	bool Initialize() { return true; }

	// 終了処理
	void Finalize();

	//****************************************************
	// data
	//****************************************************
	useful::up<CScene>    m_upScene;         // 現在のシーン
	std::function<bool()> m_fpDelayBehavior; // 遅延処理
};

// テンプレート実装ファイル
#include "API.scene.manager.tpp"