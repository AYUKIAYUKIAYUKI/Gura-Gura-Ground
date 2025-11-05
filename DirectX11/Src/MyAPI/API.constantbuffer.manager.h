
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// 定数バッファマネージャー、ヘッダーファイル [constantbuffer.manager.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.singleton.h"

//****************************************************
// 定数バッファマネージャークラスの定義
//****************************************************
class CConstantBufferManager final : public CSingleton<CConstantBufferManager>
{
	//****************************************************
	// フレンド宣言
	//****************************************************
	friend struct std::default_delete<CConstantBufferManager>;
	friend CConstantBufferManager& CSingleton<CConstantBufferManager>::RefInstance();

public:

	//****************************************************
	// バッファタイプ列挙型の定義
	//****************************************************
	enum class BufferType : unsigned char
	{
		UTIL = 0,
		MAX
	};

	//****************************************************
	// 汎用定数バッファの構造体
	//****************************************************
	struct alignas(16) CB_Util
	{
		float             fTime; // 時間
		DirectX::XMFLOAT2 Mouse; // マウス位置
		float             Padding;
	};

	//****************************************************
	// WVP行列のみの定数バッファ情報の構造体
	//****************************************************
	struct alignas(16) CB_WVP
	{
		DirectX::XMMATRIX WVP;
	};

	//****************************************************
	// カラーのみの定数バッファ情報の構造体
	//****************************************************
	struct alignas(16) CB_Col
	{
		DirectX::XMFLOAT4 Col;
	};

	//****************************************************
	// function
	//****************************************************

	// 汎用定数バッファの更新
	void UpdateGenericConstantBuffer();

	// 定数バッファの生成
	// あくまで渡されたポインタに生成するので、マネージャーが保有することを保障しません
	static bool CreateConstantbuffer(const UINT nBufferSize, ID3D11Buffer** ppConstantBuffer);

	//****************************************************
	// inline function
	//****************************************************

	// 汎用定数バッファの取得
	inline const ComPtr<ID3D11Buffer>& RefConstantBuffer(unsigned char wType) const { return m_vspConstantBuffer[wType]; }
	inline const ComPtr<ID3D11Buffer>& RefConstantBuffer(BufferType Type)     const { return m_vspConstantBuffer[static_cast<unsigned char>(Type)]; }

	// 定数バッファの更新
	// 渡されたバッファをマップして、データを送ってアンマップするところまで
	template <typename T>
	static inline void UpdateConstantBuffer(ID3D11DeviceContext* pContext, ID3D11Buffer* pConstantBuffer, const T& Data);

private:

	//****************************************************
	// special function
	//****************************************************
	CConstantBufferManager();           // デフォルトコンストラクタ
	~CConstantBufferManager() override; // デストラクタ

	//****************************************************
	// function
	//****************************************************

	// 初期化処理
	bool Initialize();

	// 終了処理
	void Finalize();

	//****************************************************
	// data
	//****************************************************

	// 汎用定数バッファのコンテナ
	std::array<ComPtr<ID3D11Buffer>, static_cast<unsigned char>(BufferType::MAX)> m_vspConstantBuffer;

	// 時間
	float m_fTime;
};

// テンプレート実装ファイル
#include "API.constantbuffer.manager.tpp"	