
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// レンダラー、ヘッダーファイル [renderer.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.singleton.h"
#include "API.camera.h"

// 各描画処理に必要なマネージャー
#include "API.vertexshader.manager.h"
#include "API.inputlayout.manager.h"
#include "API.pixelshader.manager.h"
#include "API.constantbuffer.manager.h"

//****************************************************
// コンセプトの定義
//****************************************************

// 更新可能であることのコンセプト
template <typename T>
concept Updatable = requires(T t)
{
	{ t->Draw() } -> std::same_as<void>;
};

// 更新可能コンテナであることのコンセプト
template <typename T>
concept UpdatableContainer = requires(T t)
{
	{ *t.begin() } -> Updatable;
};

// 描画可能であることのコンセプト
template <typename T>
concept Drawable = requires(T t)
{
	{ t->Draw() } -> std::same_as<void>;
};

// 描画可能コンテナであることのコンセプト
template <typename T>
concept DrawableContainer = requires(T t)
{
	{ *t.begin() } -> Drawable;
};

//****************************************************
// レンダラークラスの定義
//****************************************************
class CRenderer final : public CSingleton<CRenderer>
{
	//****************************************************
	// 前方宣言
	//****************************************************
	class CRendererImpl;

	//****************************************************
	// フレンド宣言
	//****************************************************
	friend struct std::default_delete<CRenderer>;
	friend CRenderer& CSingleton<CRenderer>::RefInstance();
	
public:

	//****************************************************
	// 深度テストタイプの列挙型を定義
	//****************************************************
	enum class DepthType : unsigned char
	{
		DEFAULT = 0, // 通常の比較方法
		NONUSE,      // 深度テストしない
		MAX
	};

	//****************************************************
	// ブレンドタイプの列挙型を定義
	//****************************************************
	enum class BlendType : unsigned char
	{
		DEFAULT = 0, // 通常のブレンド方法
		ADD,         // 加算合成
		SUB,         // 減算合成
		MAX
	};

	//****************************************************
	// function
	//****************************************************

	// 初期化処理
	bool Initialize();

	// 終了処理
	void Finalize();

	// 更新処理
	void Update(std::function<void()>&& fp);

	// 描画処理
	void Draw(std::function<void()>&& fp);

	// デバイスの取得
	      ID3D11Device* GetDevice();
	const ID3D11Device* GetDeviceConst() const;

	// デバイスコンテキストの取得
	      ID3D11DeviceContext* GetContext();
	const ID3D11DeviceContext* GetContextConst() const;

	// スワップチェインの取得
	      IDXGISwapChain* GetSwapChain();
	const IDXGISwapChain* GetSwapChainConst() const;

	// 深度ステンシルビューの取得
	      ID3D11DepthStencilView* GetDepthStencilView();
	const ID3D11DepthStencilView* GetDepthStencilViewConst() const;

	// ラスタライザーステートの取得
	      ID3D11RasterizerState* GetRasterizerState();
	const ID3D11RasterizerState* GetRasterizerStateConst() const;

	// サンプラーステートの取得
	      ID3D11SamplerState* GetSamplerState();
	const ID3D11SamplerState* GetSamplerStateConst() const;

	// カメラの取得
	CCamera* GetCamera();

	// 正射影行列の取得
	const DirectX::XMMATRIX& GetMtxOrtho();

	// ウィンドウ変更サイズをセット
	void SetResizeWindow(const UINT nWidth, const UINT nHeight);

	// 深度ステンシルステートの取得
	      ID3D11DepthStencilState* GetDepthStencilState(const DepthType Type);
	const ID3D11DepthStencilState* GetDepthStencilStateConst(const DepthType Type) const;

	// ブレンドステートの取得
	      ID3D11BlendState* GetBlendState(const BlendType Type);
	const ID3D11BlendState* GetBlendStateConst(const BlendType Type) const;

private:

	//****************************************************
	// special function
	//****************************************************
	CRenderer();           // デフォルトコンストラクタ
	~CRenderer() override; // デストラクタ

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<CRendererImpl> m_upImpl;
};