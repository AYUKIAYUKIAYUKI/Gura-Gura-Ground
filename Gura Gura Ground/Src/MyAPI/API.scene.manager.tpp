//============================================================================
// 
// シーンマネージャー、テンプレート実装ファイル [scene.manager.tpp]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//============================================================================
// 初期化処理
//============================================================================
template <DerivedFromScene T>
bool CSceneManager::Initialize()
{
	// 最初のシーンを生成
	useful::up<T> upInitScene = std::make_unique<T>();

	// 生成失敗
	if (!upInitScene)
	{
		return false;
	}

	// 出来たインスタンスをセット
	m_upScene = std::move(upInitScene);

	return true;
}

//============================================================================
// シーン変更
//============================================================================
template <DerivedFromScene T>
void CSceneManager::ChangeScene(useful::up<T>&& upScene)
{
	// 渡されたシーンに異常がある
	if (!upScene)
	{
		throw std::runtime_error("シーン遷移に引き渡された、新しいシーンが不正");
	}

	// 新しいシーンをセット
	m_upScene = std::move(upScene);
}

//============================================================================
// シーントランジション
//============================================================================
template <typename T>
void CSceneManager::TransitionScene(T&& fpDelayBehavior)
{
	// 遅延処理が渡されていない
	if (!fpDelayBehavior)
	{
		return;
	}

	// 既に保有している遅延処理が無ければ関数をホールド
	if (!m_fpDelayBehavior)
	{
		m_fpDelayBehavior = fpDelayBehavior;
	}
}