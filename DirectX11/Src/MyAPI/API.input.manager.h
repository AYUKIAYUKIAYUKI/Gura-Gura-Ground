
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// インプットマネージャー、ヘッダーファイル [input.manager.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.singleton.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "GamePad.h"

//****************************************************
// インプットマネージャークラスの定義
//****************************************************
class CInputManager final : public CSingleton<CInputManager>
{
	//****************************************************
	// フレンド宣言
	//****************************************************
	friend struct std::default_delete<CInputManager>;
	friend CInputManager& CSingleton<CInputManager>::RefInstance();

	//****************************************************
	// 静的メンバ定数の定義 (非公開)
	//****************************************************
	static constexpr unsigned char MAX_GAMEPAD = 4;

	//****************************************************
	// エイリアスを定義
	//****************************************************
	using GamePadTracker = std::array<DirectX::GamePad::ButtonStateTracker, MAX_GAMEPAD>;

public:

	//****************************************************
	// function
	//****************************************************
	
	// 更新処理
	void Update();

	//****************************************************
	// inline function
	//****************************************************

	// お手軽エンター
	bool EnhancedEnter()
	{
		// キーボードのEnterキーか
		if (m_KeyboardTracker.pressed.Enter)
		{
			return true;
		}

		// 好きなコントローラーのA, B, X, Y, START, BACKのボタンどれか 
		for (unsigned char wIdxGamePad = 0; wIdxGamePad < MAX_GAMEPAD; ++wIdxGamePad)
		{
			if (m_aGamePadTracker[wIdxGamePad].a     == DirectX::GamePad::ButtonStateTracker::PRESSED ||
				m_aGamePadTracker[wIdxGamePad].b     == DirectX::GamePad::ButtonStateTracker::PRESSED ||
				m_aGamePadTracker[wIdxGamePad].x     == DirectX::GamePad::ButtonStateTracker::PRESSED ||
				m_aGamePadTracker[wIdxGamePad].y     == DirectX::GamePad::ButtonStateTracker::PRESSED ||
				m_aGamePadTracker[wIdxGamePad].start == DirectX::GamePad::ButtonStateTracker::PRESSED ||
				m_aGamePadTracker[wIdxGamePad].back  == DirectX::GamePad::ButtonStateTracker::PRESSED)
			{
				return true;
			}
		}

		return false;
	}

	// キーボード入力用のメッセージハンドラー
	inline void HandlerKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (m_upKeyboard)
		{
			m_upKeyboard->ProcessMessage(uMsg, wParam, lParam);
		}
	}

	// マウス入力用のメッセージハンドラー
	inline void HandlerMouse(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (m_upMouse)
		{
			m_upMouse->ProcessMessage(uMsg, wParam, lParam);
		}
	}

	// キーボードのステート取得
	inline DirectX::Keyboard::State GetStateKeyboard() const { return m_upKeyboard->GetState(); }

	// キーボードトラッカーの取得
	inline const DirectX::Keyboard::KeyboardStateTracker& GetTrackerKeyboard() const { return m_KeyboardTracker; }

	// マウスのステート取得
	inline DirectX::Mouse::State GetStateMouse() { return m_upMouse->GetState(); }

	// マウスの移動量取得
	inline DirectX::XMFLOAT2 GetMoveMouse() { return m_MouseMove; }

	// マウストラッカーの取得
	inline const DirectX::Mouse::ButtonStateTracker& GetTrackerMouse() const { return m_MouseTracker; }

	// ゲームパッドのステート取得
	inline DirectX::GamePad::State GetStateGamePad(unsigned char wPadIdx = 0) { return m_upGamePad->GetState(wPadIdx); }

	// ゲームパッドトラッカーの取得
	inline const DirectX::GamePad::ButtonStateTracker& GetTrackerGamePad(unsigned char wPadIdx = 0) { return m_aGamePadTracker[wPadIdx]; }

private:

	//****************************************************
	// special function
	//****************************************************
	CInputManager();           // デフォルトコンストラクタ
	~CInputManager() override; // デストラクタ

	//****************************************************
	// function
	//****************************************************
	bool Initialize();      // 初期化処理
	void Finalize();        // 終了処理
	void UpdateMouseMove(); // マウス移動量の更新処理
	void UpdateTracker();   // トラッカーの更新処理

	//****************************************************
	// data
	//****************************************************
	std::unique_ptr<DirectX::Keyboard>      m_upKeyboard;      // キーボード
	DirectX::Keyboard::KeyboardStateTracker m_KeyboardTracker; // キーボードトラッカー
	std::unique_ptr<DirectX::Mouse>         m_upMouse;         // マウス
	DirectX::XMFLOAT2                       m_MousePosPrev;    // マウス過去位置
	DirectX::XMFLOAT2                       m_MouseMove;       // マウス移動量
	DirectX::Mouse::ButtonStateTracker      m_MouseTracker;    // マウストラッカー
	std::unique_ptr<DirectX::GamePad>       m_upGamePad;       // ゲームパッド
	GamePadTracker                          m_aGamePadTracker; // ゲームパッドトラッカー
};