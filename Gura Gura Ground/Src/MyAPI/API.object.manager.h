
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// オブジェクトマネージャー、ヘッダーファイル [object.manager.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.singleton.h"
#include "API.object.h"

//****************************************************
// オブジェクトマネージャークラスを定義
//****************************************************
class CObjectManager : public CSingleton<CObjectManager>
{
	//****************************************************
	// フレンド宣言
	//****************************************************
	friend struct std::default_delete<CObjectManager>;
	friend CObjectManager& CSingleton<CObjectManager>::RefInstance();

	//****************************************************
	// 型エイリアスを定義
	//****************************************************
	using Object_Share = std::shared_ptr<CObject>;

	using List_Raw = std::list<CObject*>;
	using List_Share = std::list<Object_Share>;

	//****************************************************
	// 静的メンバ定数を定義 (非公開)
	//****************************************************

	// 許容するオブジェクトの最大数
	template <typename T = unsigned char>
	static constexpr T MAX_OBJECT = (std::numeric_limits<T>::max)();

public:

	//****************************************************
	// special funciton
	//****************************************************
	CObjectManager();           // デフォルトコンストラクタ
	~CObjectManager() override; // デストラクタ

	//****************************************************
	// funciton
	//****************************************************

	// 初期化処理
	bool Initialize();

	// 終了処理
	void Finalize();

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	// 全オブジェクトの死亡フラグを立てる
	void SetDeathAll();

	// 生成処理
	template <DerivedFromObject T>
	static T* CreateRaw(OBJ::TYPE Type = OBJ::TYPE::NONE, OBJ::LAYER Layer = OBJ::LAYER::DEFAULT);

#if CONCEPT_ε || CONCEPT_ρ
	template <DerivedFromObject T, ObjectFactoryFunction U>
	static std::shared_ptr<T> CreateRaw(U&& fpFactory, OBJ::TYPE Type = OBJ::TYPE::NONE, OBJ::LAYER Layer = OBJ::LAYER::DEFAULT);
#else
	template <DerivedFromObject T, typename U>
	static T* CreateRaw(U&& fpFactory, OBJ::TYPE Type = OBJ::TYPE::NONE, OBJ::LAYER Layer = OBJ::LAYER::DEFAULT);
#endif

	template <DerivedFromObject T>
	static std::shared_ptr<T> CreateShare(OBJ::TYPE Type = OBJ::TYPE::NONE, OBJ::LAYER Layer = OBJ::LAYER::DEFAULT);

#if CONCEPT_ε || CONCEPT_ρ
	template <DerivedFromObject T, ObjectFactoryFunction U>
	static std::shared_ptr<T> CreateShare(U&& fpFactory, OBJ::TYPE Type = OBJ::TYPE::NONE, OBJ::LAYER Layer = OBJ::LAYER::DEFAULT);
#else
	template <DerivedFromObject T, typename U>
	static std::shared_ptr<T> CreateShare(U&& fpFactory, OBJ::TYPE Type = OBJ::TYPE::NONE, OBJ::LAYER Layer = OBJ::LAYER::DEFAULT);
#endif

	// 生ポインタのオブジェクトリストを走査する高階関数
	template<typename T>
	void ForEachRaw(const T& fpExecute);

	// シェアポインタのオブジェクトリストを走査する高階関数
	template<typename T>
	void ForEachShare(const T& fpExecute);

	//****************************************************
	// inline function
	//****************************************************

	// 新規オブジェクトの登録
	inline void RegisterObjectRaw(CObject*&& pObject) { m_aListRaw[static_cast<unsigned char>(pObject->GetType())].push_back(pObject); }
	inline void RegisterObjectShare(Object_Share spObject) { m_aListShare[static_cast<unsigned char>(spObject->GetType())].push_back(spObject); }

	// オブジェクトのリストを取得
	inline const std::array<List_Raw, static_cast<unsigned char>(OBJ::TYPE::MAX)>& RefListRaw() { return m_aListRaw; }
	inline const std::array<List_Share, static_cast<unsigned char>(OBJ::TYPE::MAX)>& RefListShare() { return m_aListShare; }

	// 指定したタイプのオブジェクトのリストを取得
	inline const List_Raw& RefListRaw(OBJ::TYPE Type) { return m_aListRaw[static_cast<unsigned char>(Type)]; }
	inline const List_Share& RefListShare(OBJ::TYPE Type) { return m_aListShare[static_cast<unsigned char>(Type)]; }

private:

	//****************************************************
	// function
	//****************************************************
	void UpdateListRaw();       // 生ポインタのオブジェクト一斉更新
	void LateUpdateListRaw();   // 生ポインタのオブジェクト一斉遅延更新
	void UpdateListShare();     // シェアポインタのオブジェクト一斉更新
	void LateUpdateListShare(); // シェアポインタのオブジェクト一斉遅延更新

	//****************************************************
	// data
	//****************************************************

	// 保有する全てのタイプ分のリスト
	std::array<List_Raw, static_cast<unsigned char>(OBJ::TYPE::MAX)> m_aListRaw;
	std::array<List_Share, static_cast<unsigned char>(OBJ::TYPE::MAX)> m_aListShare;
};

//============================================================================
// 生成処理
//============================================================================
template <DerivedFromObject T>
T* CObjectManager::CreateRaw(OBJ::TYPE Type, OBJ::LAYER Layer)
{
	// オブジェクトの生成
	T* pObj = DBG_NEW T(Type, Layer);

	// 生成失敗
	if (!pObj)
	{
		throw std::runtime_error("オブジェクトの生成に失敗");
	}

	RefInstance().RegisterObjectRaw(pObj);

	return pObj;
}

//============================================================================
// 生成処理
//============================================================================
#if CONCEPT_ε || CONCEPT_ρ
template <DerivedFromObject T, ObjectFactoryFunction U>
T* CObject::Create(U&& fpFactory, OBJ::TYPE Type, OBJ::LAYER Layer)
#else
template <DerivedFromObject T, typename U>
T* CObjectManager::CreateRaw(U&& fpFactory, OBJ::TYPE Type, OBJ::LAYER Layer)
#endif
{
	// オブジェクトの生成
	T* pObj = DBG_NEW T(Type, Layer);

	// 生成失敗
	if (!pObj)
	{
		throw std::runtime_error("オブジェクトの生成に失敗");
	}

	// 渡されたファクトリでセットアップ
	if (!fpFactory(pObj))
	{
		throw std::runtime_error("オブジェクトのセットアップに失敗");
	}

	RefInstance().RegisterObjectRaw(pObj);

	return pObj;
}

//============================================================================
// 生成処理
//============================================================================
template <DerivedFromObject T>
std::shared_ptr<T> CObjectManager::CreateShare(OBJ::TYPE Type, OBJ::LAYER Layer)
{
	// オブジェクトの生成
	std::shared_ptr<T> spObj = std::make_shared<T>(Type, Layer);

	// 生成失敗
	if (!spObj)
	{
		throw std::runtime_error("オブジェクトの生成に失敗");
	}

	RefInstance().RegisterObjectShare(spObj);

	return spObj;
}

//============================================================================
// 生成処理
//============================================================================
#if CONCEPT_ε || CONCEPT_ρ
template <DerivedFromObject T, ObjectFactoryFunction U>
std::shared_ptr<T> CObjectManager::Create(U&& fpFactory, OBJ::TYPE Type, OBJ::LAYER Layer)
#else
template <DerivedFromObject T, typename U>
std::shared_ptr<T> CObjectManager::CreateShare(U&& fpFactory, OBJ::TYPE Type, OBJ::LAYER Layer)
#endif
{
	// オブジェクトの生成
	std::shared_ptr<T> spObj = std::make_shared<T>(Type, Layer);

	// 生成失敗
	if (!spObj)
	{
		throw std::runtime_error("オブジェクトの生成に失敗");
	}

	// 渡されたファクトリでセットアップ
	if (!fpFactory(spObj.get()))
	{
		throw std::runtime_error("オブジェクトのセットアップに失敗");
	}

	RefInstance().RegisterObjectShare(spObj);

	return spObj;
}

// 生ポインタのオブジェクトリストを走査する高階関数
template<typename T>
void CObjectManager::ForEachRaw(const T& fpCallBack)
{
	// 生ポインタのオブジェクトのリストを取得
	const std::array<std::list<CObject*>, static_cast<unsigned char>(OBJ::TYPE::MAX)>& rListRaw = CObjectManager::RefInstance().RefListRaw();

	// 全てのリストを走査
	for (const std::list<CObject*>& rTypeList : rListRaw)
	{
		for (const auto& rIt : rTypeList)
		{
			fpCallBack(rIt);
		}
	}
}

// シェアポインタをのオブジェクトリストを走査する高階関数
template<typename T>
void CObjectManager::ForEachShare(const T& fpCallBack)
{
	// 共有ポインタのオブジェクトのリストを取得
	const std::array<std::list<std::shared_ptr<CObject>>, static_cast<unsigned char>(OBJ::TYPE::MAX)>& rListShare = CObjectManager::RefInstance().RefListShare();

	// 全てのリストを走査
	for (const std::list<std::shared_ptr<CObject>>& rTypeList : rListShare)
	{
		for (const std::shared_ptr<CObject>& rIt : rTypeList)
		{
			fpCallBack(rIt);
		}
	}
}