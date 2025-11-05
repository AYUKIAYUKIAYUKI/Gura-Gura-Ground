
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
	using ObjList = std::list<CObject*>;

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
	bool Initialize() { return true; }

	// 終了処理
	void Finalize() {}

	// 保有する全オブジェクトの更新
	void UpdateAllObject();

	// 保有する全オブジェクトの遅延更新
	// 死亡フラグの対処なんかもここ
	void LateUpdateAllObject();

	// 保有する全オブジェクトに対して死亡フラグを立てる
	void SetDeathAllObject();

	// 保有する全オブジェクトの描画
	void DrawAllObject();

	//****************************************************
	// inline function
	//****************************************************

	// 新規オブジェクトの登録
	inline void RegisterObj(CObject*&& pObj) { m_aAllObjList[static_cast<unsigned char>(pObj->GetType())].push_back(pObj); }

	// 指定したタイプのオブジェクトのリストを取得
	inline const ObjList& RefObjList(OBJ::TYPE Type) { return m_aAllObjList[static_cast<unsigned char>(Type)]; }

	// オブジェクトのリストを取得
	inline const std::array<ObjList, static_cast<unsigned char>(OBJ::TYPE::MAX)>& RefObjList() { return m_aAllObjList; }

private:

	//****************************************************
	// data
	//****************************************************

	// 保有する全てのタイプ分のリスト
	std::array<ObjList, static_cast<unsigned char>(OBJ::TYPE::MAX)> m_aAllObjList;
};