//===============================================================================
//
//  エフェクトの管理(effect.manager.cpp)
//								制作：元地弘汰
// 
//===============================================================================
#include "effect.manager.h"

#include "API.renderer.h"


namespace {
    static int SelectNum = 0;
    std::vector<std::string> TagName;
    void EditUI()
    {
#ifdef _DEBUG
        ImGui::SetNextWindowPos(ImVec2(1000, 0));
        ImGui::SetNextWindowSize(ImVec2(300, 400));
        ImGui::Begin("Effect Edit");

        if(ImGui::Button("Instance"))CEffectManager::RefInstance();
        if (ImGui::Button("Thunder"))CEffect::Create(CEffectManager::TAG_LIGHTNING, {20.0f,0.0f,0.0f});
        ImGui::SameLine();
        if (ImGui::Button("Water"))CEffect::Create(L"Data\\EFFECT\\Effect\\Simple_Turbulence_Fireworks.efkefc", { -20.0f,0.0f,0.0f },nullptr,1.0f);
        if (ImGui::Button("FireWork"))CEffect::Create(CEffectManager::TAG_FIREWORKS_SINGLE, { 0.0f,-20.0f,0.0f });

        ImGui::Button("<");
        ImGui::SameLine();
        ImGui::Button(">");

        ImGui::End();
#endif // _DEBUG
    }
}

CEffect::~CEffect()
{
    Uninit();
}

//==========================================================================================
//初期化処理
//==========================================================================================
void CEffect::Init()
{
    CEffectManager::RefInstance().RegistEffect(this);
    Effekseer::ManagerRef m_manager = CEffectManager::RefInstance().GetManager();
    Effekseer::Vector3D a = { m_pos.x,m_pos.y,m_pos.z };

    m_handle = m_manager->Play(m_effects, a);
}
//==========================================================================================
//終了処理
//==========================================================================================
void CEffect::Uninit()
{
    if (m_effects != nullptr)    {
        m_effects.Reset();

    }
}

//==========================================================================================
//更新処理
//==========================================================================================
void CEffect::Update()
{
    if (m_bPlaying)return;
    Effekseer::ManagerRef m_manager = CEffectManager::RefInstance().GetManager();
    Effekseer::Vector3D a = { m_pos.x,m_pos.y,m_pos.z };

    if(m_effects == nullptr)throw std::runtime_error("Effect is nullptr");
    if (!m_manager->Exists(m_handle))
    {
        m_manager->StopEffect(m_handle);
        m_bPlaying = false;
    }
}

//==========================================================================================
//描画処理
//==========================================================================================
void CEffect::Draw()
{
    
}

//==========================================================================================
//生成処理
//==========================================================================================
CEffect* CEffect::Create(const std::wstring& filename, useful::Vec3 pos,int* handle,float sizevalue)
{
    CEffect* pEffect = DBG_NEW CEffect();
    Effekseer::ManagerRef m_manager = CEffectManager::RefInstance().GetManager();
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.good())throw std::runtime_error("Effect file not found!!!");
    pEffect->m_effects = Effekseer::Effect::Create(m_manager, (char16_t*)(filename.c_str()), sizevalue);
    if(pEffect->m_effects == nullptr)throw std::runtime_error("Effect is nullptr!!!!!!");
    pEffect->m_pos = pos;
    pEffect->Init();
    if (handle != nullptr)handle = &pEffect->m_handle;
    return pEffect;
}
//==========================================================================================
//生成処理
//==========================================================================================
CEffect* CEffect::Create(CEffectManager::EFFECT_TAG tag, useful::Vec3 pos, int* handle, float sizevalue)
{
    CEffect* pEffect = DBG_NEW CEffect();
    Effekseer::ManagerRef m_manager = CEffectManager::RefInstance().GetManager();

    std::string filename = CEffectManager::RefInstance().GetFilename(tag);
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.good())throw std::runtime_error("Effect file not found!!!");
    // UTF-8 → UTF-16 の長さ取得
    int len = MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, nullptr, 0);

    // 変換先の UTF-16 バッファ（wstring）
    std::wstring wstr(len, u'\0');

    // stringからwstringに変換の変換
    MultiByteToWideChar(
        CP_UTF8, 0,
        filename.c_str(), -1,
        &wstr[0], len
    );

    pEffect->m_effects = Effekseer::Effect::Create(m_manager, (char16_t*)(wstr.c_str()), sizevalue);

    if (pEffect->m_effects == nullptr)throw std::runtime_error("Effect is nullptr!!!!!!");

    pEffect->m_pos = pos;
    pEffect->Init();
    if (handle != nullptr)handle = &pEffect->m_handle;
    return pEffect;
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------
//ここから下は管理クラス
CEffectManager::CEffectManager()
{

}

CEffectManager::~CEffectManager()
{
}

//==========================================================================================
//初期化処理
//==========================================================================================
bool CEffectManager::Initialize()
{
    ID3D11Device* pDevice = CRenderer::RefInstance().GetDevice();
    ID3D11DeviceContext* pContext = CRenderer::RefInstance().GetContext();
    // Effekseerレンダラー作成
    m_renderer = EffekseerRendererDX11::Renderer::Create(pDevice, pContext,2000);
    if (m_renderer == nullptr)throw std::runtime_error("Effecseer renderer Error");

    // Effekseerマネージャ作成
    m_manager = Effekseer::Manager::Create(2000);
    if(m_manager == nullptr)throw std::runtime_error("Effecseer manager Error");
    // 各種レンダラーを登録
    m_manager->SetSpriteRenderer(m_renderer->CreateSpriteRenderer());
    m_manager->SetRibbonRenderer(m_renderer->CreateRibbonRenderer());
    m_manager->SetRingRenderer(m_renderer->CreateRingRenderer());
    m_manager->SetTrackRenderer(m_renderer->CreateTrackRenderer());
    m_manager->SetModelRenderer(m_renderer->CreateModelRenderer());

    m_manager->SetTextureLoader(m_renderer->CreateTextureLoader());
    m_manager->SetModelLoader(m_renderer->CreateModelLoader());
    m_manager->SetMaterialLoader(m_renderer->CreateMaterialLoader());

    m_manager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
    // 状態復帰フラグ（DirectXの状態を戻す）
    m_renderer->SetRestorationOfStatesFlag(true);

    LoadFile();

    return true;
}

//==========================================================================================
//終了処理
//==========================================================================================
void CEffectManager::Finalize()
{
    m_manager->StopAllEffects();
    for (auto& e : m_effectsList)
    {
        delete e;
    }
    m_effectsList.clear();
    if (m_manager != nullptr) {
        m_manager.Reset();
    }
    if (m_renderer != nullptr)    {
        m_renderer.Reset();
    }
    
}

//==========================================================================================
//更新処理
//==========================================================================================
void CEffectManager::Update()
{
    m_EffectName;
    EditUI();
    m_manager->Update();    //エフェクシアのマネージャーを更新.
    //一括で更新
    for (auto& i : m_effectsList) {
        i->Update();
    }
    EraseEffect();
}

//==========================================================================================
//描画処理
//==========================================================================================
void CEffectManager::Draw()
{
    SetCameraMtx();

    //エフェクシアのレンダリング
    m_renderer->BeginRendering();
    m_manager->Draw();
    m_renderer->EndRendering();
}

//==========================================================================================
//マネージャーに登録
//==========================================================================================
void CEffectManager::RegistEffect(CEffect* peff)
{
    m_effectsList.push_back(peff);      //リストに登録
}

//==========================================================================================
//マネージャーから削除
//==========================================================================================
void CEffectManager::EraseEffect()
{
    for (const auto& e : m_effectsList)
    {
        //実行済みかどうか確認
        if (e->GetPlaying())continue;
        //リストから除外(アクセス違反を防ぐためErase-removeイディオムを使用)
        m_effectsList.erase(std::remove(m_effectsList.begin(), m_effectsList.end(),e));
    }
}

//==========================================================================================
//リストから検索
//==========================================================================================
CEffect* CEffectManager::GetEffect(int handle)
{
    for (auto& e : m_effectsList)
    {
        if (e->GetHandle() == handle)return e;
    }
    return nullptr;
}

bool CEffectManager::LoadFile()
{
    std::ifstream ifs(LoadFilename);    //ファイルの読み込み
    if (!ifs) {
        throw std::runtime_error("Cannot Found JsonFile "); 
        return false;
    }
    nlohmann::ordered_json jsonFile;
   
    ifs >> jsonFile;

    for (auto& obj : jsonFile["EffectList"])
    {
        for (auto& e : obj.items())
        {
            std::string keyname = e.key();
            TagName.push_back(keyname);
            std::string valuename = e.value().get<std::string>();
            m_EffectName.push_back(valuename);
        }
    }

    return true;
}


//==========================================================================================
//カメラのマトリックスを設定
//==========================================================================================
void CEffectManager::SetCameraMtx()
{
    //カメラのビューと投影の行列を取得
    DirectX::CXMMATRIX CamViewMtx = CRenderer::RefInstance().GetCamera()->GetView();
    DirectX::CXMMATRIX CamProjMtx = CRenderer::RefInstance().GetCamera()->GetProjection();

    Effekseer::Matrix44 viewmtx;
    DirectX::XMFLOAT4X4 StoredViewMtx;
    DirectX::XMStoreFloat4x4(&StoredViewMtx, CamViewMtx);       //ビュー行列をFloat4X4型に変換


    memcpy(viewmtx.Values, &StoredViewMtx, sizeof(float) * 16); //エフェクシア側の型にコピー

    Effekseer::Matrix44 prtjmtx;
    DirectX::XMFLOAT4X4 StoredProjMtx;
    DirectX::XMStoreFloat4x4(&StoredProjMtx, CamProjMtx);       //投影行列をFloat4X4型に変換

    memcpy(prtjmtx.Values, &StoredProjMtx, sizeof(float) * 16); //エフェクシア側の型にコピー

    EffekseerRendererDX11::RendererRef renderer = CEffectManager::RefInstance().GetRenderer();

    renderer->SetCameraMatrix(viewmtx);
    renderer->SetProjectionMatrix(prtjmtx);
}