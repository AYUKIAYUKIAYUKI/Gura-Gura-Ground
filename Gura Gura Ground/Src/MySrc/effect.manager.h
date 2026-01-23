//===============================================================================
//
//  エフェクトの管理(effect.manager.h)
//								制作：元地弘汰
// 
//===============================================================================
#ifndef _EFFECT_MANAGER_H_
#define _EFFECT_MANAGER_H_

#include "API.useful.h"
#include "API.singleton.h"

class CEffect;
class CEffectManager : public CSingleton<CEffectManager>
{
private:
	CEffectManager();	//コンストラクタ
	~CEffectManager();	//デストラクタ

	bool Initialize();		//初期化

	void SetCameraMtx();
	void EraseEffect();
	// Effekseer関連のスマートポインタ
	EffekseerRendererDX11::RendererRef m_renderer;
	Effekseer::ManagerRef m_manager;

	// 読み込まれたエフェクトリスト
	std::vector<CEffect*> m_effectsList;

	//****************************************************
	// フレンド宣言
	//****************************************************
	friend struct std::default_delete<CEffectManager>;
	friend CEffectManager& CSingleton<CEffectManager>::RefInstance();


	const std::string LoadFilename = "Data\\JSON\\Effect.json";
	std::vector<std::string> m_EffectName;
public:
	enum EFFECT_TAG
	{
		TAG_LIGHTNING = 0,
		TAG_WATER,
		TAG_FIREWORKS,
		TAG_HIPDROP,
		TAG_FIREWORKS_SINGLE,
		TAG_TORNADE,
		TAG_BOMB,
		TAG_FLASH,
		TAG_SMOKE
	};
	void Update();			//更新
	void Draw();			//描画

	void RegistEffect(CEffect* pEffect);
	bool LoadFile();
	void Finalize();		//終了

	std::string GetFilename(EFFECT_TAG tag) { if (m_EffectName.size() > tag) return m_EffectName[tag]; }
	void StopAll() { m_manager->StopAllEffects(); }

	inline Effekseer::ManagerRef GetManager() { return m_manager; }
	inline EffekseerRendererDX11::RendererRef GetRenderer() { return m_renderer; };
	CEffect* GetEffect(int handle);
};

class CEffect
{
public:
	CEffect() :m_bPlaying(true), m_handle(-1), m_pos({0.0f,0.0f,0.0f}) {};	//コンストラクタ
	~CEffect();			//デストラクタ
	void Init();		//初期化
	void Uninit();		//終了
	void Update();		//更新
	void Draw();		//描画

	bool GetPlaying() { return m_bPlaying; }
	int GetHandle() { return m_handle; }
	void SetDeath();

	void SetLocation(useful::Vec3 pos) { m_pos = pos; }

	static CEffect* Create(const std::wstring& filename, useful::Vec3 pos,int* handle = nullptr,float sizevalue = 1.0f);	//ファイル名、座標、ハンドルのポインタ(任意)、拡大率(任意)
	static CEffect* Create(CEffectManager::EFFECT_TAG tag, useful::Vec3 pos, int* handle = nullptr, float sizevalue = 1.0f);	//ファイル名、座標、ハンドルのポインタ(任意)、拡大率(任意)

private:

	bool m_bPlaying;
	useful::Vec3 m_pos;

	//エフェクシア側の変数
	Effekseer::EffectRef m_effects;		//エフェクトのポインタ
	Effekseer::Handle	 m_handle;		//エフェクトのハンドル
};

#endif