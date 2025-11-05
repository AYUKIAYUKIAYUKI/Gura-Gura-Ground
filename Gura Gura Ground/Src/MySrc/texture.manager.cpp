//============================================================================
// 
// テクスチャマネージャー [texture.manager.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "texture.manager.h"
#include <WICTextureLoader.h>

// デバイス取得用
#include "API.renderer.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CTextureManager::CTextureManager()
	: m_upRegistry(std::make_unique<CRegistry<ComPtr<ID3D11ShaderResourceView>>>())
{}

//============================================================================
// デストラクタ
//============================================================================
CTextureManager::~CTextureManager()
{
	// 終了処理
	Finalize();

	// 通過ログ
	OutputDebugStringW(L"dtor：テクスチャマネージャー\n");
}

//============================================================================
// 初期化処理
//============================================================================
bool CTextureManager::Initialize()
{
	// 生成処理
	std::function<ComPtr<ID3D11ShaderResourceView>(const std::string&)> fpCreate =
		[](const std::string& Path) -> ComPtr<ID3D11ShaderResourceView>
	{
		// APIの戻り値を格納
		HRESULT hr = S_OK;

		// テクスチャのポインタ
		ComPtr<ID3D11ShaderResourceView> pElement;

		// デバイスを取得
		auto pDev = CRenderer::RefInstance().GetDevice();

		// デバイスコンテキストを取得
		auto pCtx = CRenderer::RefInstance().GetContext();

		// ？
		std::wstring wFilePath(Path.begin(), Path.end());

		// GPUリソースとしてテクスチャを生成
		hr = DirectX::CreateWICTextureFromFile(
			pDev,
			pCtx,
			wFilePath.c_str(),
			nullptr,
			pElement.GetAddressOf());

		// 生成失敗
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to Create Texture" + Path);
		}

		// テクスチャのポインタを返す
		return pElement;
	};

	// 解放処理
	std::function<void(ComPtr<ID3D11ShaderResourceView>&)> fpRelease =
		[](ComPtr<ID3D11ShaderResourceView>& /*pElement*/) -> void
	{
		// 特にクリーンアップしない
	};

	// レジストリの初期化処理
	if (!m_upRegistry->Initialize(INITIALIZE_PATH, std::move(fpCreate), std::move(fpRelease)))
	{
		return false;
	}

	return true;
}

//============================================================================
// 終了処理
//============================================================================
void CTextureManager::Finalize()
{
	// レジストリの終了処理
	m_upRegistry->Finalize();
}