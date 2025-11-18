
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// オブジェクト、テンプレート実装ファイル [object.tpp]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//============================================================================
// 生成処理
//============================================================================
template <DerivedFromObject T>
T* CObject::Create(OBJ::TYPE Type, OBJ::LAYER Layer)
{
	// オブジェクトの生成
	T* pObj = DBG_NEW T(Type, Layer);

	// 生成失敗
	if (!pObj)
	{
		throw std::runtime_error("オブジェクトの生成に失敗");
	}

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
T* CObject::Create(U&& fpFactory, OBJ::TYPE Type, OBJ::LAYER Layer)
#endif
{
	// 型名を保持
	//const char* pTyneName = +typeid(T).name();

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

	return pObj;
}