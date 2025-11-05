//============================================================================
// 
// サウンドマネージャー、ヘッダーファイル [sound.manager.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "API.singleton.h"
#include <Audio.h>

//****************************************************
// サウンドマネージャークラスの定義
//****************************************************
class CSoundManger final : public CSingleton<CSoundManger>
{
	//****************************************************
	// フレンド宣言
	//****************************************************
	friend struct std::default_delete<CSoundManger>;
	friend CSoundManger& CSingleton<CSoundManger>::RefInstance();

	//****************************************************
	// 静的メンバ変数の定義
	//****************************************************
	static constexpr const char* INITIALIZE_PATH = "Data\\JSON\\Sound.List.json";

public:

	//****************************************************
	// function
	//****************************************************

	// サウンドを再生
	inline void Play(std::string sKey, bool bLoop = false, float fPitch = 0.0f, float fVolume = 1.0f)
	{
		// すでに再生状態にある場合は一度停止
		if (m_mupSoundInstance.find(sKey)->second->GetState() == DirectX::SoundState::PLAYING)
		{
			m_mupSoundInstance.find(sKey)->second->Stop();
		}

		m_mupSoundInstance.find(sKey)->second->Play(bLoop);
		m_mupSoundInstance.find(sKey)->second->SetPitch(fPitch);
		m_mupSoundInstance.find(sKey)->second->SetVolume(fVolume);
	}
	
	// ピッチを設定
	inline void SetPitch(std::string sKey, float fPitch) { m_mupSoundInstance.find(sKey)->second->SetPitch(fPitch); }

	// ボリュームを設定
	inline void SetVolume(std::string sKey, float fVolume) { m_mupSoundInstance.find(sKey)->second->SetVolume(fVolume); }

private:

	//****************************************************
	// special function
	//****************************************************
	CSoundManger();           // デフォルトコンストラクタ
	~CSoundManger() override; // デストラクタ

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
	std::unique_ptr<DirectX::AudioEngine>                                m_upAudioEngine;    // オーディオエンジン
	std::vector<std::unique_ptr<DirectX::SoundEffect>>                   m_vupSound;         // サウンドのファイルデータ
	std::map<std::string, std::unique_ptr<DirectX::SoundEffectInstance>> m_mupSoundInstance; // サウンドのインスタンス
};