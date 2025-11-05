
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// マネージャークラステンプレート、ヘッダーファイル [manager.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.singleton.h"
#include "API.registry.h"

//****************************************************
// マネージャークラステンプレートの定義
//****************************************************
template <typename T, typename U>
class CManager : public CSingleton<T>
{
public:

	//****************************************************
	// inline function
	//****************************************************

	// レジストリの参照の取得
	inline CRegistry<U>& RefRegistry(const char* pAny = "Any Registry") const { return *useful::PtrCheck(m_upRegistry.get(), pAny); }

protected:

	//****************************************************
	// inline function
	//****************************************************

	// 派生のマネージャーがレジストリのセットアップを行えるようにするラッパー
	inline bool CallRegistryInitializeForDerived(const std::string& Path, std::function<U(const std::string&)>&& fpFactory, std::function<void(U&)>&& fpRelease)
	{
		return m_upRegistry->Initialize(Path, std::move(fpFactory), std::move(fpRelease));
	}

	//****************************************************
	// special function
	//****************************************************

	// デフォルトコンストラクタ
	CManager()
		: m_upRegistry(std::make_unique<CRegistry<U>>())
	{}

	// デストラクタ
	virtual ~CManager() override {}

private:

	//****************************************************
	// function
	//****************************************************

	 // 初期化処理
	virtual bool Initialize() { return true; }

	// 終了処理
	virtual void Finalize() {} 

	//****************************************************
	// data
	//****************************************************

	// レジストリ
	std::unique_ptr<CRegistry<U>> m_upRegistry;
};