
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// ウィンドウ、テンプレート実装ファイル [window.tpp]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//============================================================================
// メッセージループ
//============================================================================
template <typename T>
void CWindow::MessageLoop(T&& fpMainLoop)
{
	// FPS計測用
	int   nCountFPS      = 0;             // 現在のFPS
	DWORD dwCurrentTime  = 0;             // 現在時刻用
	DWORD dwFrameCount   = 0;             // 最後に処理した時間
	DWORD dwExecLastTime = timeGetTime(); // フレームカウント格納
	DWORD dwFPSLastTime  = timeGetTime(); // 最後にFPSを計測した時刻格納

	while (true)
	{
		// メッセージを確認
		if (PeekMessage(&m_Msg, NULL, 0, 0, PM_REMOVE))
		{
			// WM_QUITのメッセージを受け取ると、メッセージループを抜ける
			if (m_Msg.message == WM_QUIT)
			{
				break;
			}

			// メッセージの設定
			TranslateMessage(&m_Msg); // 仮想キーメッセージを文字メッセージへ変換
			DispatchMessage(&m_Msg);  // ウィンドウプロシージャへメッセージを送出
		}
		else
		{
			// 現在時刻を取得
			dwCurrentTime = timeGetTime();

			if ((dwCurrentTime - dwFPSLastTime) >= 500)
			{
				// FPSを計測
				nCountFPS = (dwFrameCount * 1000) / (dwCurrentTime - dwFPSLastTime);

				// FPSを計測した時間を保存
				dwFPSLastTime = dwCurrentTime;

				// フレームカウントをクリア
				dwFrameCount = 0;
			}

			if ((dwCurrentTime - dwExecLastTime) >= (1000 / 60))
			{
				// 現在時刻を保存
				dwExecLastTime = dwCurrentTime;

				// メインループ実行
				fpMainLoop();

				// フレームカウントを加算
				++dwFrameCount;
			}
		}
	}
}