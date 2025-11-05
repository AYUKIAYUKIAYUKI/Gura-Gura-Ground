
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// シングルトンクラステンプレート、テンプレート実装ファイル [singleton.tpp]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// 静的メンバ変数の初期化
//****************************************************

// インスタンスのファクトリ
template <typename T>
std::function<std::unique_ptr<T>()> CSingleton<T>::s_fpFactory = nullptr;

// インスタンスのポインタ
template <typename T>
std::unique_ptr<T> CSingleton<T>::s_upInstance = nullptr;

//============================================================================
// インスタンスの参照を取得
//============================================================================
template <typename T>
T& CSingleton<T>::RefInstance()
{
	// 初回生成以降、インスタンスの参照を渡す
	if (s_upInstance)
	{
		return *s_upInstance;
	}

#if 1 // 派生クラス側がこのメソッドをfriend指定するなら… 
	
	// インスタンス未生成の場合のみ、新規生成
	std::unique_ptr<T> upInstance(DBG_NEW T());
	s_upInstance = std::move(upInstance);
	
	// 引数無しの初期化処理のみ自動的に行う
	if (!s_upInstance->Initialize())
	{
		s_upInstance->Finalize();
	}
#else // これなら、内部でメモリ確保と初期化を済ませてポインタを受け取るだけ

	if (s_fpFactory)
	{
		s_upInstance = s_fpFactory();
	}
#endif

	return *s_upInstance;
}