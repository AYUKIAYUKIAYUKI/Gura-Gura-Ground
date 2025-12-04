//============================================================================
// 
// HUDエディター [Hudeditor.cpp]
// Author : Sohta Kuki
// 
//============================================================================

#include "Hudeditor.h"
#include <fstream>
#include <algorithm>
#include "imgui.h"

// デフォルトテクスチャキー
const char* CHudEditor::DefaultKeys[3] = { "Uchiyama", "Frame", "Logo.A" };

//============================================================================
// コンストラクタ
//============================================================================
CHudEditor::CHudEditor() 
{
   //テクスチャキーを取得
   std::ifstream tj("Data\\JSON\\Texture.List.json");
   if (tj) {
       nlohmann::json tjroot;
       tj >> tjroot;
       for (const auto& elem : tjroot["Element"]) 
       {
           vTextureKeys.push_back(elem[0].get<std::string>());
       }
   }

   // 万が一ファイルが読めなければデフォルトに
   if (vTextureKeys.empty()) 
   {
       vTextureKeys.assign(DefaultKeys, DefaultKeys + 3);
   }

    // JSONファイルがある場合
    if (std::ifstream("Data\\JSON\\HudSettings.json").good()) 
    {
        LoadFromFile("Data\\JSON\\HudSettings.json");
    }
    else 
    {
        // JSONファイルがない場合
        float windowCenterX = OBJ::CalcCenterOfWindow().x;
        for (int hudIndex = 0; hudIndex < 3; ++hudIndex)
        {
            auto hud = CObject::Create<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
            auto tex = CTextureManager::RefInstance().RefRegistry().BindAtKey(DefaultKeys[hudIndex]);
            hud->SetTexture(tex);
            OBJ::Transform tf = { {300.0f, 300.0f, 0.0f}, {0,0,0,0}, {windowCenterX + hudIndex * 320.0f - 320.0f, 100.0f, 0.0f} };
            DirectX::XMFLOAT4 col = { 1,1,1,1 };
            hud->SetTransform(tf);
            hud->SetTransformTarget(tf);
            hud->SetColTarget(col);

            vHudUI.push_back(hud);           // HUDオブジェクト
            vTf.push_back(tf);               // 座標・サイズ等
            vCol.push_back(col);             // カラー
            vHudNames.push_back(DefaultKeys[hudIndex]); // テクスチャキー
            vVisible.push_back(true);
            vDisplayTime.push_back(0.0f);  // 0は常時表示
        }
    }
}

//============================================================================
// デストラクタ
//============================================================================
CHudEditor::~CHudEditor()
{}

//============================================================================
// 更新処理
//============================================================================
void CHudEditor::Update()
{
#ifdef _DEBUG
    // HUDが無ければ何も表示しない
    if (vHudUI.empty()) return;

    //Imguiウィンドウの座標、サイズ指定
    static bool firstWindow = true;
    if (firstWindow) 
    {
        ImGui::SetNextWindowPos(ImVec2(825, 5), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(440, 500), ImGuiCond_FirstUseEver);
        firstWindow = false;
    }

    ImGui::Begin("HUD Edit List");

    std::vector<const char*> texKeyCstrs;
    for (auto& key : vTextureKeys) texKeyCstrs.push_back(key.c_str());

    // HUDごとにImGuiで編集項目を表示
    for (int hudIndex = 0; hudIndex < vHudUI.size(); ++hudIndex)
    {
        ImGui::PushID(hudIndex);
        ImGui::Separator();
        ImGui::Text("HUD #%d : %s", hudIndex + 1, vHudNames[hudIndex].c_str());
        // レイヤーを1つ上に移動
        if (hudIndex > 0 && ImGui::Button("Layer UP"))
        {
            MoveHudUp(hudIndex);
            ImGui::PopID();
            break;
        }
        ImGui::SameLine();
        // レイヤーを1つ下に移動
        if (hudIndex < vHudUI.size() - 1 && ImGui::Button("Layer DOWN"))
        {
            MoveHudDown(hudIndex);
            ImGui::PopID();
            break;
        }

        // テクスチャ選択コンボ
        int curTexIdx = 0;
        for (int t = 0; t < (int)vTextureKeys.size(); ++t)
            if (vTextureKeys[t] == vHudNames[hudIndex]) curTexIdx = t;
        if (ImGui::Combo("Texture", &curTexIdx, texKeyCstrs.data(), (int)texKeyCstrs.size()))
        {
            vHudNames[hudIndex] = vTextureKeys[curTexIdx];
            if (vHudUI[hudIndex])
            {
                auto tex = CTextureManager::RefInstance().RefRegistry().BindAtKey(vHudNames[hudIndex].c_str());
                vHudUI[hudIndex]->SetTexture(tex);
            }
        }

        // 位置とサイズを調整
        ImGui::DragFloat2("Pos", &vTf[hudIndex].Pos.x, 1.0f);
        ImGui::DragFloat2("Size", &vTf[hudIndex].Size.x, 1.0f);
        //向きを編集
        ImGui::DragFloat("Rot", &vTf[hudIndex].Rot.z, 1.0f);
        // 色を編集
        ImGui::ColorEdit4("Color", &vCol[hudIndex].x);

        if (vHudUI[hudIndex])
        {
            vHudUI[hudIndex]->SetTransformTarget(vTf[hudIndex]);
            vHudUI[hudIndex]->SetColTarget(vCol[hudIndex]);
        }

        bool Visibletmp = (vVisible[hudIndex] != 0);
        if (ImGui::Checkbox("Visible", &Visibletmp))
        {
            vVisible[hudIndex] = Visibletmp ? 1 : 0;
        }

        ImGui::DragFloat("Display Time", &vDisplayTime[hudIndex], 0.1f, 0.0f, 9999.0f);

        // HUD削除ボタン
        if (ImGui::Button("Delete"))
        {
            RemoveHud(hudIndex);
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }
    // HUDの追加ボタン
    if (ImGui::Button("Add New HUD"))
    {
        int defaultKeyIndex = vHudUI.size() % 3;
        AddHud(DefaultKeys[defaultKeyIndex]);
    }

    // 現在の設定をJSONに保存
    if (ImGui::Button("Save Param"))
    {
        SaveToFile("Data\\JSON\\HudSettings.json");
    }
    ImGui::End();

#endif
}


//============================================================================
// HUDを追加
//============================================================================
void CHudEditor::AddHud(const std::string& textureKey)
{
    // ウィンドウ中央座標を取得し新しいHUDを生成
    float WCX = OBJ::CalcCenterOfWindow().x;
    auto hud = CObject::Create<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
    auto tex = CTextureManager::RefInstance().RefRegistry().BindAtKey(textureKey.c_str());
    hud->SetTexture(tex);
    OBJ::Transform tf = { {300.0f, 300.0f, 0.0f}, {0,0,0,0}, {WCX, 100.0f, 0.0f} };
    DirectX::XMFLOAT4 col = { 1,1,1,1 };
    hud->SetTransform(tf);
    hud->SetTransformTarget(tf);
    hud->SetColTarget(col);

    // 各種情報を配列に追加
    vHudUI.push_back(hud);
    vTf.push_back(tf);
    vCol.push_back(col);
    vHudNames.push_back(textureKey);
    vVisible.push_back(true);
    vDisplayTime.push_back(0.0f);  // 0は常時表示

    // レイヤー情報更新
    ReFlashHudObjects();
}

//============================================================================
// 既存のHUDを削除
//============================================================================
void CHudEditor::RemoveHud(int idx)
{
    // Deleteフラグを立てて各種情報を削除
    if (vHudUI[idx]) vHudUI[idx]->SetDeath();
    vHudUI.erase(vHudUI.begin() + idx);
    vTf.erase(vTf.begin() + idx);
    vCol.erase(vCol.begin() + idx);
    vHudNames.erase(vHudNames.begin() + idx);
    ReFlashHudObjects();
}

//============================================================================
// 描画順切り替え(上)
//============================================================================
void CHudEditor::MoveHudUp(int idx) 
{
    if (idx > 0)
    {
        std::swap(vHudUI[idx], vHudUI[idx - 1]);
        std::swap(vTf[idx], vTf[idx - 1]);
        std::swap(vCol[idx], vCol[idx - 1]);
        std::swap(vHudNames[idx], vHudNames[idx - 1]);
        ReFlashHudObjects();
    }
}

//============================================================================
// 描画順切り替え(下)
//============================================================================
void CHudEditor::MoveHudDown(int idx)
{
    if (idx < (int)vHudUI.size() - 1)
    {
        std::swap(vHudUI[idx], vHudUI[idx + 1]);
        std::swap(vTf[idx], vTf[idx + 1]);
        std::swap(vCol[idx], vCol[idx + 1]);
        std::swap(vHudNames[idx], vHudNames[idx + 1]);
        ReFlashHudObjects();
    }
}

//============================================================================
// HUDパラメーター保存
//============================================================================
void CHudEditor::SaveToFile(const std::string& path) 
{
    nlohmann::json savejson;

    // 現在登録済みHUDの全情報をJSON配列化
    for (size_t hudIndex = 0; hudIndex < vHudUI.size(); ++hudIndex)
    {
        nlohmann::json item;
        item["texture_name"] = vHudNames[hudIndex];
        item["pos"] = { vTf[hudIndex].Pos.x, vTf[hudIndex].Pos.y, vTf[hudIndex].Pos.z };
        item["size"] = { vTf[hudIndex].Size.x, vTf[hudIndex].Size.y, vTf[hudIndex].Size.z };
        item["rot"] = { vTf[hudIndex].Rot.x, vTf[hudIndex].Rot.y, vTf[hudIndex].Rot.z, vTf[hudIndex].Rot.w };
        item["color"] = { vCol[hudIndex].x, vCol[hudIndex].y, vCol[hudIndex].z, vCol[hudIndex].w };
        item["visible"] = vVisible[hudIndex];
        item["display_time"] = vDisplayTime[hudIndex];
        item["layer_index"] = hudIndex;
        savejson.push_back(item);
    }
    // ファイルへ書き込み
    std::ofstream out(path);
    out << savejson.dump(4);
}

//============================================================================
// HUDパラメーター読み込み
//============================================================================
void CHudEditor::LoadFromFile(const std::string& path)
{
    std::ifstream in(path);
    if (!in) return;

    nlohmann::json loadjson;
    in >> loadjson;

    // 一時的にHUD情報を格納する構造体
    struct HudInfo
    {
        size_t layer_index;
        std::string texName;
        OBJ::Transform tf;
        DirectX::XMFLOAT4 col;
        bool visible;
        float displayTime;
    };

    std::vector<HudInfo> infos;
    // JSONの各項目を解析してHUD情報を取得
    for (const auto& item : loadjson)
    {
        HudInfo info;
        info.texName = item["texture_name"].get<std::string>();
        info.tf.Pos = { item["pos"][0], item["pos"][1], item["pos"][2] };
        info.tf.Size = { item["size"][0], item["size"][1], item["size"][2] };
        info.tf.Rot = { item["rot"][0], item["rot"][1], item["rot"][2], item["rot"][3] };
        info.col = { item["color"][0], item["color"][1], item["color"][2], item["color"][3] };
        info.visible = item.value("visible", true);
        info.displayTime = item.value("display_time", 0.0f);
        info.layer_index = item.value("layer_index", infos.size());
        infos.push_back(info);
    }
    // レイヤー順でソート
    std::sort(infos.begin(), infos.end(), [](const HudInfo& a, const HudInfo& b)
        {
            return a.layer_index < b.layer_index;
        });

    // 現在のHUD情報をクリアし、ファイル情報で再構築
    vHudUI.clear(); vTf.clear(); vCol.clear(); vHudNames.clear();
    for (const auto& info : infos)
    {
        auto hud = CObject::Create<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
        auto tex = CTextureManager::RefInstance().RefRegistry().BindAtKey(info.texName.c_str());
        hud->SetTexture(tex);
        hud->SetTransform(info.tf);
        hud->SetTransformTarget(info.tf);
        hud->SetColTarget(info.col);
        vHudUI.push_back(hud);
        vTf.push_back(info.tf);
        vCol.push_back(info.col);
        vHudNames.push_back(info.texName);
        vVisible.push_back(info.visible);
        vDisplayTime.push_back(info.displayTime);
    }
}

//============================================================================
// HUDパラメーター読み込み
//============================================================================
void CHudEditor::ReFlashHudObjects()
{
    // 既存のHUDオブジェクトを全て削除
    for (auto* hud : vHudUI)
        if (hud) hud->SetDeath();

    // 再生成用の新配列
    std::vector<CHud*> newList;

    for (int hudIndex = 0; hudIndex < vHudNames.size(); ++hudIndex)
    {
        auto hud = CObject::Create<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);

        auto tex = CTextureManager::RefInstance()
            .RefRegistry().BindAtKey(vHudNames[hudIndex].c_str());
        hud->SetTexture(tex);

        hud->SetTransform(vTf[hudIndex]);
        hud->SetTransformTarget(vTf[hudIndex]);
        hud->SetColTarget(vCol[hudIndex]);

        newList.push_back(hud);
    }

    vHudUI = newList;
}
