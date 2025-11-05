//============================================================================
// 
// サウンドマネージャー [sound.manager.cpp]
// Author : 福田歩希
// 
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "sound.manager.h"

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CSoundManger::CSoundManger()
	: m_upAudioEngine(nullptr)
	, m_vupSound()
	, m_mupSoundInstance()
{}

//============================================================================
// デストラクタ
//============================================================================
CSoundManger::~CSoundManger()
{
	// 終了処理
	Finalize();

	// 通過ログ
	OutputDebugStringW(L"dtor：サウンドマネージャー\n");
}

//============================================================================
// 初期化処理
//============================================================================
bool CSoundManger::Initialize()
{
	// オーディオエンジンの生成
	m_upAudioEngine = std::make_unique<DirectX::AudioEngine>();

	if (!m_upAudioEngine)
	{
		return false;
	}

	// 初期化リストを展開
	const JSON& SoundList = useful::OpenJsonFileMaybeThrow(INITIALIZE_PATH);

	// 要素数を取得
	int nSize = static_cast<int>(SoundList["Element"].size());

	for (int nIdxSound = 0; nIdxSound < nSize; ++nIdxSound)
	{
		// サウンドのデータへのパスをワイド文字列に変換
		std::string  sFilePath = SoundList["Element"][nIdxSound][1].get<std::string>().c_str();
		std::wstring wFilePath(sFilePath.begin(), sFilePath.end());

		// サウンドのデータファイルを展開、一応保有しておく
		m_vupSound.push_back(std::make_unique<DirectX::SoundEffect>(m_upAudioEngine.get(), wFilePath.c_str()));

		// 保有したデータからサウンドをインスタンス化
		std::unique_ptr<DirectX::SoundEffectInstance> upSoundInstance = m_vupSound[nIdxSound]->CreateInstance();

		// キーとインスタンスをペアとして保有
		m_mupSoundInstance.insert(std::make_pair(SoundList["Element"][nIdxSound][0].get<std::string>(), std::move(upSoundInstance)));
	}

	return true;
}

//============================================================================
// 終了処理
//============================================================================
void CSoundManger::Finalize()
{}