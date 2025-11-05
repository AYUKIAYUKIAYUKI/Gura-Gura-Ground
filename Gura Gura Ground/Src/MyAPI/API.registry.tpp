
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// レジストリクラステンプレート、テンプレート実装ファイル [registry.tpp]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//============================================================================
// デフォルトコンストラクタ
//============================================================================
template <typename T>
CRegistry<T>::CRegistry()
	: m_fpFactory(nullptr)
	, m_fpRelease(nullptr)
	, m_mData{}
{}

//============================================================================
// デストラクタ
//============================================================================
template <typename T>
CRegistry<T>::~CRegistry()
{
	// 終了処理
	Finalize();
}

//============================================================================
// 初期化処理
//============================================================================
template <typename T>
bool CRegistry<T>::Initialize(const std::string& Path, std::function<T(const std::string&)>&& fpFactory, std::function<void(T&)>&& fpRelease)
{
	// 生成か破棄の関数が無い場合初期化出来ない
	if (!fpFactory || !fpRelease)
	{
		return false;
	}

	// 受け取った関数をムーブ
	m_fpFactory = std::move(fpFactory);
	m_fpRelease = std::move(fpRelease);

	// 指定パスのJSONファイルを展開
	const JSON& Json = useful::OpenJsonFileMaybeThrow(Path);

	// データ数を確認
	const decltype(Json["Element"].size()) MaxSize = Json["Element"].size();

	// データの生成を行い、マップに格納する
	for (int nCntIdx = 0; nCntIdx < MaxSize; ++nCntIdx)
	{
		m_mData.insert(std::make_pair(Json["Element"][nCntIdx][0], m_fpFactory(Json["Element"][nCntIdx][1])));
	}

	return true;
}

//============================================================================
// 終了処理
//============================================================================
template <typename T>
void CRegistry<T>::Finalize()
{
	// 解放用の関数が無い
	if (m_fpRelease)
	{
		std::runtime_error("Kaihouyou Kansu ga Nai");
	}

	// 事前に受け取っていた関数で全てのデータを解放
	for (auto& it : m_mData)
	{
		m_fpRelease(it.second);
	}

	// マップをクリア
	m_mData.clear();
}

//============================================================================
// キーを受け取って、保有している物を返す
//============================================================================
template <typename T>
T CRegistry<T>::BindAtKey(const std::string& Key)
{
	// 指定されたキーに対応するデータを探す
	const decltype(m_mData)::iterator& it = m_mData.find(Key);

	// 見つからなかった場合は例外を投げる
	if (it == m_mData.end())
	{
		throw std::runtime_error("'BindAtKey' was Failed : " + Key);
	}

	// 見つかった場合は対応するデータを返す
	return it->second;
}

//============================================================================
// ランタイム向けの新規登録
//============================================================================
template <typename T>
bool CRegistry<T>::Register(const std::string& Key, const std::string& Path)
{
	// 生成用の関数が無い
	if (m_fpFactory)
	{
		std::runtime_error("Seiseiyou Kansu ga Nai");
	}

	// 指定されたキーに対応するデータを探す
	const decltype(m_mData)::iterator& it = m_mData.find(Key);

	// 既に登録されている場合は登録出来ない
	if (it != m_mData.end())
	{
		return false;
	}

	// 新規登録
	m_mData.insert(std::make_pair(Key, m_fpFactory(Path)));

	// 登録の成功を通知する
	return true;
}