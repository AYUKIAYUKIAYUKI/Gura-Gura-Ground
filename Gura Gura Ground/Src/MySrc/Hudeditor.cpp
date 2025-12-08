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
            vDisplayDelay.push_back(0.0f); // 表示遅延
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

    if (bPreviewMode)
    {
        double currentTime = ImGui::GetTime(); // ImGui時間を取得

        for (size_t i = 0; i < vHudUI.size(); ++i)
        {
            double elapsed = currentTime - vShowStartTime[i];

            // 表示開始時間を未設定なら初期化
            if (vShowStartTime.size() <= i) vShowStartTime.resize(vHudUI.size(), currentTime);
            if (vShowStartTime[i] == 0.0) vShowStartTime[i] = currentTime;

            // 表示時間を超えていれば消滅させる
            if (vDisplayTime[i] > 0.0f &&
                currentTime - vShowStartTime[i] >= vDisplayTime[i])
            {
                if (vHudUI[i])
                {
                    vCol[i].w = 0.0f;
                    vVisible[i] = false;
                    vHudUI[i]->SetColTarget(vCol[i]);
                }
                continue;
            }

            if (elapsed < vDisplayDelay[i]) 
            {
                // 描画遅延中はHUDを非表示にする
                if (vHudUI[i]) 
                {
                    vCol[i].w = 0.0f;
                    vVisible[i] = false;
                    vHudUI[i]->SetColTarget(vCol[i]);
                }
                continue;
            }
            else 
            {
                // 遅延終了後は表示（アルファ値を1.0fに設定する）
                if (vHudUI[i]) 
                {
                    vCol[i].w = 1.0f;
                    vVisible[i] = true;
                    vHudUI[i]->SetColTarget(vCol[i]);
                }
            }


            // vVisibleがfalseなら非表示
            if (!vVisible[i] && vHudUI[i])
            {
                vCol[i].w = 0.0f;
                vHudUI[i]->SetColTarget(vCol[i]);
            }
        }
        // nullptrのHUDを削除
        auto itHud = vHudUI.begin();
        auto itTf = vTf.begin();
        auto itCol = vCol.begin();
        auto itName = vHudNames.begin();
        auto itVisible = vVisible.begin();
        auto itDisplay = vDisplayTime.begin();
        auto itStartTime = vShowStartTime.begin();

        for (size_t idx = 0; idx < vHudUI.size(); )
        {
            if (vHudUI[idx] == nullptr)
            {
                itHud = vHudUI.erase(itHud);
                itTf = vTf.erase(itTf);
                itCol = vCol.erase(itCol);
                itName = vHudNames.erase(itName);
                itVisible = vVisible.erase(itVisible);
                itDisplay = vDisplayTime.erase(itDisplay);
                itStartTime = vShowStartTime.erase(itStartTime);
            }
            else
            {
                ++itHud; ++itTf; ++itCol; ++itName; ++itVisible; ++itDisplay; ++itStartTime; ++idx;
            }
        }
    }

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

    // プレビューモード切替UI追加
    bool preChecked = bPreviewMode;
    if (ImGui::Checkbox("Preview Mode", &preChecked))
    {
        SetSwitchMode(preChecked);
    }

    // HUDごとにImGuiで編集項目を表示
    for (int hudIndex = 0; hudIndex < vHudUI.size(); ++hudIndex)
    {
        ImGui::PushID(hudIndex);
        ImGui::Separator();

        std::string headerTitle = "HUD #" + std::to_string(hudIndex + 1) + " : " + vHudUserNames[hudIndex];
        if (ImGui::CollapsingHeader(headerTitle.c_str(), ImGuiTreeNodeFlags_None))
        {
            // プレビューモード中にHUDの残り表示タイマーを表示
            if (bPreviewMode)
            {
                double currentTime = ImGui::GetTime();
                double timer = 0.0;
                double startTime = (vShowStartTime.size() > hudIndex) ? vShowStartTime[hudIndex] : currentTime;
                double elapsed = currentTime - startTime;

                if (elapsed < vDisplayDelay[hudIndex])
                {
                    double delayLeft = vDisplayDelay[hudIndex] - elapsed;
                    if (delayLeft < 0.0) delayLeft = 0.0;
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Display start left : %.2f sec", delayLeft);
                }
                else
                {
                    if (vDisplayTime[hudIndex] > 0.0f)
                    {
                        double DisplayStartTime = (vShowStartTime.size() > hudIndex) ? vShowStartTime[hudIndex] : currentTime;
                        timer = vDisplayTime[hudIndex] - (currentTime - DisplayStartTime);
                        if (timer < 0.0) timer = 0.0;
                        {
                            ImGui::TextColored(ImVec4(1, 0.5f, 0.1f, 1), "Display end left: %.2f sec", timer);
                        }
                    }
                    else
                    {
                        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.2f, 1), "Always visible");
                    }
                }
            }

            //プレビューモード中は表示させない
            if (!bPreviewMode)
            {
                //HUDの名称を設定
                constexpr size_t BUF_SIZE = 64;
                char userNameBuf[BUF_SIZE] = {};
                auto& srcStr = vHudUserNames[hudIndex];
                size_t len = (srcStr.size() < BUF_SIZE - 1) ? srcStr.size() : (BUF_SIZE - 1);
                std::copy(srcStr.begin(), srcStr.begin() + len, userNameBuf);
                userNameBuf[len] = '\0';
                if (ImGui::InputText("HUD Name", userNameBuf, BUF_SIZE))
                {
                    vHudUserNames[hudIndex] = userNameBuf;
                }

                // レイヤーを1つ上に移動
                if (hudIndex > 0 && ImGui::Button("Layer UP"))
                {
                    MoveHudUp(hudIndex);
                    ImGui::PopID();
                    break;
                }
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

                ImGui::DragFloat("Display Time", &vDisplayTime[hudIndex], 0.1f, 0.0f, 10000.0f);
                ImGui::DragFloat("Display Delay", &vDisplayDelay[hudIndex], 0.1f, 0.0f, 10000.0f);

                // HUD削除ボタン
                if (ImGui::Button("Delete"))
                {
                    RemoveHud(hudIndex);
                    ImGui::PopID();
                    break;
                }
            }
        }
        ImGui::PopID();
    }

    // HUDの追加ボタン
    if (ImGui::Button("Add New HUD"))
    {
        AddHud();
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
void CHudEditor::AddHud()
{
    float WCX = OBJ::CalcCenterOfWindow().x;
    std::string firstTexKey;
    if (!vTextureKeys.empty()) {
        firstTexKey = vTextureKeys[0];
    }
    else {
        firstTexKey = DefaultKeys[0];
    }
    auto hud = CObject::Create<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
    auto tex = CTextureManager::RefInstance().RefRegistry().BindAtKey(firstTexKey.c_str());
    hud->SetTexture(tex);
    OBJ::Transform tf = { {300.0f, 300.0f, 0.0f}, {0,0,0,0}, {WCX, 100.0f, 0.0f} };
    DirectX::XMFLOAT4 col = { 1,1,1,1 };
    hud->SetTransform(tf);
    hud->SetTransformTarget(tf);
    hud->SetColTarget(col);

    vHudUI.push_back(hud);
    vTf.push_back(tf);
    vCol.push_back(col);
    vHudNames.push_back(firstTexKey);
    vHudUserNames.push_back("New HUD");
    vVisible.push_back(true);
    vDisplayTime.push_back(0.0f);
    vDisplayDelay.push_back(0.0f);

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
    vHudUserNames.erase(vHudUserNames.begin() + idx);
    vDisplayDelay.erase(vDisplayDelay.begin() + idx);
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
        std::swap(vHudUserNames[idx], vHudUserNames[idx - 1]);
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
        std::swap(vHudUserNames[idx], vHudUserNames[idx + 1]);
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
        item["hud_name"] = vHudUserNames[hudIndex];
        item["texture_name"] = vHudNames[hudIndex];
        item["pos"] = { vTf[hudIndex].Pos.x, vTf[hudIndex].Pos.y, vTf[hudIndex].Pos.z };
        item["size"] = { vTf[hudIndex].Size.x, vTf[hudIndex].Size.y, vTf[hudIndex].Size.z };
        item["rot"] = { vTf[hudIndex].Rot.x, vTf[hudIndex].Rot.y, vTf[hudIndex].Rot.z, vTf[hudIndex].Rot.w };
        item["color"] = { vCol[hudIndex].x, vCol[hudIndex].y, vCol[hudIndex].z, vCol[hudIndex].w };
        item["visible"] = vVisible[hudIndex];
        item["display_time"] = vDisplayTime[hudIndex];
        item["display_delay"] = vDisplayDelay[hudIndex];
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

    std::vector<HudInfo> infos;
    // JSONの各項目を解析してHUD情報を取得
    for (const auto& item : loadjson)
    {
        HudInfo info;
        info.hudName = item.value("hud_name", info.texName);
        info.texName = item["texture_name"].get<std::string>();
        info.tf.Pos = { item["pos"][0], item["pos"][1], item["pos"][2] };
        info.tf.Size = { item["size"][0], item["size"][1], item["size"][2] };
        info.tf.Rot = { item["rot"][0], item["rot"][1], item["rot"][2], item["rot"][3] };
        info.col = { item["color"][0], item["color"][1], item["color"][2], item["color"][3] };
        info.visible = item.value("visible", true);
        info.displayTime = item.value("display_time", 0.0f);
        info.displayDelay = item.value("display_delay", 0.0f);
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
        vHudUserNames.push_back(info.hudName);
        vTf.push_back(info.tf);
        vCol.push_back(info.col);
        vHudNames.push_back(info.texName);
        vVisible.push_back(info.visible);
        vDisplayTime.push_back(info.displayTime);
        vDisplayDelay.push_back(info.displayDelay);
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

    if (vDisplayDelay.size() < vHudUI.size())
        vDisplayDelay.resize(vHudUI.size(), 0.0f);

    vHudUI = newList;
}

//============================================================================
// モード切り替え時処理
//============================================================================
void CHudEditor::SetSwitchMode(bool enabled)
{
    if (bPreviewMode == enabled) return;

    if (enabled)
    {
        //プレビューモードエントリー時
        vBackupHud.clear();
        for (size_t i = 0; i < vHudUI.size(); ++i) 
        {
            vBackupHud.push_back({
                vHudNames[i],
                vTf[i],
                vCol[i],
                vVisible[i],
                vDisplayTime[i]
                });
        }
        // 表示時間スタートも初期化
        double nowTime = ImGui::GetTime();
        vShowStartTime.resize(vHudUI.size(), nowTime);
    }
    else
    {
        //プレビューから移行する前に削除されてないHUDを消す
        for (auto* hud : vHudUI) 
        {
            if (hud) hud->SetDeath();
        }

        vHudUI.clear();
        vTf.clear();
        vCol.clear();
        vHudNames.clear();
        vVisible.clear();
        vDisplayTime.clear();
        vShowStartTime.clear();

        for (const auto& b : vBackupHud)
        {
            auto hud = CObject::Create<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);
            auto tex = CTextureManager::RefInstance().RefRegistry().BindAtKey(b.textureName.c_str());
            hud->SetTexture(tex);
            hud->SetTransform(b.tf);
            hud->SetTransformTarget(b.tf);
            hud->SetColTarget(b.col);

            vHudUI.push_back(hud);
            vTf.push_back(b.tf);
            vCol.push_back(b.col);
            vHudNames.push_back(b.textureName);
            vVisible.push_back(b.visible);
            vDisplayTime.push_back(b.displayTime);
            vShowStartTime.push_back(0.0);
        }
        vBackupHud.clear();
    }
    bPreviewMode = enabled;
}