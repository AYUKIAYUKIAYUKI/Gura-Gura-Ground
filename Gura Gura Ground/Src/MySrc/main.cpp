//============================================================================
// 
// ぐらぐら ぐらうんど [main.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************

// 公開APIの読み込み
#include "API.window.h"
#include "API.renderer.h"
#include "API.input.manager.h"
#include "API.sound.manager.h"
#include "API.texture.manager.h"
#include "API.gltf.manager.h"
#include "API.scene.manager.h"
#include "API.object.manager.h"
#include "API.world.h"

// 初期シーン生成のため
#include "scene.title.h"

//****************************************************
// エントリーポイント
//****************************************************
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hInstancePrev*/, _In_ LPSTR /*lpCmdLine*/, _In_ int /*nCmdShow*/)
{
	// CRTメモリリーク検出用
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// メインスレッドのCOMオブジェクトをマルチスレッド対応に変更
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
		return 0;
	}

	try
	{
		// ウィンドウの生成、明示的に初期化
		if (!CWindow::RefInstance().Initialize(hInstance))
		{
			throw std::runtime_error("ウィンドウの生成失敗");
		}

		// レンダラーの生成
		CRenderer::RefInstance();

		// インプットマネージャーの生成
		CInputManager::RefInstance();

		// サウンドマネージャーの生成
		CSoundManger::RefInstance();

		// テクスチャマネージャーの生成
		CTextureManager::RefInstance();

		// glTFモデルマネージャーの生成
		CGltfManager::RefInstance();

		// シーンマネージャーの生成
		CSceneManager::RefInstance().Initialize<CSceneTitle>();

		// オブジェクトマネージャーの生成
		CObjectManager::RefInstance();

		// 物理ワールドの生成
		CWorld::RefInstance();

		/* サウンドの再生 */
		CSoundManger::RefInstance().Play("BGM", true, 0.0f, 1.0f);
		CSoundManger::RefInstance().Play("Noise", true, 0.0f, 0.5f);

		// メッセージループ
		CWindow::RefInstance().MessageLoop(
			[]() -> void
			{
				// レンダラーの更新処理
				CRenderer::RefInstance().Update(
					[]() -> void
					{
						// インプットの更新
						CInputManager::RefInstance().Update();

						// シーンの更新
						CSceneManager::RefInstance().Update();

						// 物理ワールドの更新
						CWorld::RefInstance().Update();

						// オブジェクトリストの更新
						CObjectManager::RefInstance().UpdateAllObject();
						CObjectManager::RefInstance().LateUpdateAllObject();
					});

				// レンダラーの描画処理
				CRenderer::RefInstance().Draw(
					[]() -> void
					{
						// オブジェクトリストの描画
						CObjectManager::RefInstance().DrawAllObject();
					});
			});
	}
	catch (const std::exception& Error)
	{
		MessageBox(nullptr, Error.what(), "エラー発生", MB_OK | MB_ICONERROR);
	}

	// メインスレッドのCOMをクリーンアップ
	CoUninitialize();

	return 0;
}