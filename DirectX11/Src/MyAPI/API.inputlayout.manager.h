
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// 入力レイアウトマネージャー、ヘッダーファイル [inputlayout.manager.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.singleton.h"

//****************************************************
// 入力レイアウトマネージャークラスの定義
//****************************************************
class CInputLayoutManager final : public CSingleton<CInputLayoutManager>
{
	//****************************************************
	// 前方宣言
	//****************************************************
	class Impl;

	//****************************************************
	// フレンド宣言
	//****************************************************
	friend struct std::default_delete<CInputLayoutManager>;
	friend CInputLayoutManager& CSingleton<CInputLayoutManager>::RefInstance();

public:

	//****************************************************
	// 頂点タイプ列挙型の定義
	//****************************************************
	enum class VertexType : unsigned char
	{
		VERTEX_2D = 0,
		VERTEX_3D,
		MAX
	};

	//****************************************************
	// 2D用頂点情報の構造体の定義
	//****************************************************
	struct Vertex2D
	{
		DirectX::XMFLOAT3 Pos; // 頂点座標
		DirectX::XMFLOAT2 UV;  // UVマッピング
	};

	//****************************************************
	// 3D用頂点情報の構造体の定義
	//****************************************************
	struct Vertex3D
	{
		DirectX::XMFLOAT3 Pos; // 頂点座標
		DirectX::XMFLOAT3 Nor; // 法線ベクトル
		DirectX::XMFLOAT2 UV;  // UVマッピング
	};

	//****************************************************
	// function
	//****************************************************

	// 入力レイアウトの取得
	const ComPtr<ID3D11InputLayout>& RefInputLayout(const unsigned char wType) const;
	const ComPtr<ID3D11InputLayout>& RefInputLayout(const VertexType Type) const;

	//****************************************************
	// static function
	//****************************************************

	// クアッド(2D)用の頂点バッファの生成
	static bool CreateVertexBuffer2D(ID3D11Buffer** ppVertexBuffer);

	// クアッド(3D)用の頂点バッファの生成
	//static bool CreateVertexBufferForQuad3D(ID3D11Buffer** ppVertexBuffer);

	// メッシュ用の頂点バッファの生成
	static bool CreateVertexBufferForMesh(ID3D11Buffer** ppVertexBuffer, const std::vector<Vertex3D>& rvVertices);

	// メッシュ用のインデックスバッファの生成
	static bool CreateIndexBufferForMesh(ID3D11Buffer** ppIndexBuffer, const std::vector<uint16_t>& rvIndices);
	//static bool CreateIndexBufferForMesh(ID3D11Buffer** ppIndexBuffer, const std::vector<uint32_t>& rvIndices);

	// 既存メッシュから取得した頂点情報を自分の頂点フォーマットに変換
	template <typename T, typename U>
	void ConvertVertices(const T& rBase, U& rDest)
	{
		rDest.resize(rBase.size());

		for (size_t i = 0; i < rBase.size(); ++i)
		{
			rDest[i].Pos = rBase[i].position;
			rDest[i].Nor = rBase[i].normal;
			rDest[i].UV  = rBase[i].textureCoordinate;
		}
	}

private:

	//****************************************************
	// special function
	//****************************************************
	CInputLayoutManager();           // デフォルトコンストラクタ
	~CInputLayoutManager() override; // デストラクタ

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
	std::unique_ptr<Impl> m_upImpl;
};