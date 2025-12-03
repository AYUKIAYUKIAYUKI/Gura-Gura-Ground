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
    // JSONファイルがある場合
    if (std::ifstream("Data\\JSON\\HudSettings.json").good()) 
    {
        LoadFromFile("Data\\JSON\\HudSettings.json");
    }
    else 
    {
        // JSONファイルがない場合
        float WCX = OBJ::CalcCenterOfWindow().x;
        for (int i = 0; i < 3; ++i)
        {
            auto hud = CObject::Create<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
            auto tex = CTextureManager::RefInstance().RefRegistry().BindAtKey(DefaultKeys[i]);
            hud->SetTexture(tex);
            OBJ::Transform tf = { {300.0f, 300.0f, 0.0f}, {0,0,0,0}, {WCX + i * 320.0f - 320.0f, 100.0f, 0.0f} };
            DirectX::XMFLOAT4 col = { 1,1,1,1 };
            hud->SetTransform(tf);
            hud->SetTransformTarget(tf);
            hud->SetColTarget(col);

            vHudUI.push_back(hud);           // HUD本体を格納
            vTf.push_back(tf);               // 位置・サイズ情報を格納
            vCol.push_back(col);             // 色情報を格納
            vHudNames.push_back(DefaultKeys[i]); // テクスチャ名を格納
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
    ImGui::Begin("HUD Edit List");
    // HUDごとにImGuiで編集項目を表示
    for (int i = 0; i < vHudUI.size(); ++i)
    {
        ImGui::PushID(i);
        ImGui::Separator();
        ImGui::Text("HUD #%d : %s", i + 1, vHudNames[i].c_str());
        // レイヤーを1つ上に移動
        if (i > 0 && ImGui::Button("Layer UP"))
        {
            MoveHudUp(i);
            ImGui::PopID();
            break;
        }
        ImGui::SameLine();
        // レイヤーを1つ下に移動
        if (i < vHudUI.size() - 1 && ImGui::Button("Layer DOWN"))
        {
            MoveHudDown(i);
            ImGui::PopID();
            break;
        }
        // 位置とサイズを調整
        ImGui::DragFloat2("Pos", &vTf[i].Pos.x, 1.0f);
        ImGui::DragFloat2("Size", &vTf[i].Size.x, 1.0f);
        //向きを編集
        ImGui::DragFloat("Rot", &vTf[i].Rot.z, 1.0f);
        // 色を編集
        ImGui::ColorEdit4("Color", &vCol[i].x);

        if (vHudUI[i])
        {
            vHudUI[i]->SetTransformTarget(vTf[i]);
            vHudUI[i]->SetColTarget(vCol[i]);
        }
        // HUD削除ボタン
        if (ImGui::Button("Delete"))
        {
            RemoveHud(i);
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }
    // HUDの追加ボタン
    if (ImGui::Button("Add New HUD"))
    {
        int idx = vHudUI.size() % 3;
        AddHud(DefaultKeys[idx]);
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

    // レイヤー情報更新
    UpdateHudLayers();
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
    UpdateHudLayers();
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
        UpdateHudLayers();
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
        UpdateHudLayers();
    }
}

//============================================================================
// 描画順更新
//============================================================================
void CHudEditor::UpdateHudLayers() 
{
    for (int i = 0; i < vHudUI.size(); ++i)
        if (vHudUI[i]) vHudUI[i]->SetLayer(static_cast<OBJ::LAYER>(i));
}

//============================================================================
// HUDパラメーター保存
//============================================================================
void CHudEditor::SaveToFile(const std::string& path) 
{
    nlohmann::json j;
    // 現在登録済みHUDの全情報をJSON配列化
    for (size_t i = 0; i < vHudUI.size(); ++i) 
    {
        nlohmann::json item;
        item["texture_name"] = vHudNames[i];
        item["pos"] = { vTf[i].Pos.x, vTf[i].Pos.y, vTf[i].Pos.z };
        item["size"] = { vTf[i].Size.x, vTf[i].Size.y, vTf[i].Size.z };
        item["rot"] = { vTf[i].Rot.x, vTf[i].Rot.y, vTf[i].Rot.z, vTf[i].Rot.w };
        item["color"] = { vCol[i].x, vCol[i].y, vCol[i].z, vCol[i].w };
        item["layer_index"] = i;
        j.push_back(item);
    }
    // ファイルへ書き込み
    std::ofstream out(path);
    out << j.dump(4);
}

//============================================================================
// HUDパラメーター読み込み
//============================================================================
void CHudEditor::LoadFromFile(const std::string& path)
{
    std::ifstream in(path);
    if (!in) return;

    nlohmann::json j;
    in >> j;

    // 一時的にHUD情報を格納する構造体
    struct HudInfo
    {
        size_t layer_index;
        std::string texName;
        OBJ::Transform tf;
        DirectX::XMFLOAT4 col;
    };
    std::vector<HudInfo> infos;
    // JSONの各項目を解析してHUD情報を取得
    for (const auto& item : j)
    {
        HudInfo info;
        info.texName = item["texture_name"].get<std::string>();
        info.tf.Pos = { item["pos"][0], item["pos"][1], item["pos"][2] };
        info.tf.Size = { item["size"][0], item["size"][1], item["size"][2] };
        info.tf.Rot = { item["rot"][0], item["rot"][1], item["rot"][2], item["rot"][3] };
        info.col = { item["color"][0], item["color"][1], item["color"][2], item["color"][3] };
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
    }
}