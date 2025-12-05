//============================================================================
// 
// HUDエディター [Hudeditor.h]
// Author : Sohta Kuki
// 
//============================================================================

#pragma once

#include "API.hud.h"
#include "API.texture.manager.h"
#include "API.object.manager.h"

//****************************************************
// HUDエディタークラスの定義
//****************************************************
class CHudEditor 
{
public:
    // コンストラクタ
    CHudEditor();
    // デストラクタ
    ~CHudEditor();
    // 毎フレーム更新・ImGui描画
    void Update();
    // 設定をファイル保存
    void SaveToFile(const std::string& path);
    // 設定をファイルから読込
    void LoadFromFile(const std::string& path);

    // HUD追加・削除・階層変更・レイヤー同期
    void AddHud(const std::string& textureKey); // HUDを追加
    void RemoveHud(int idx);                    // HUDを削除
    void MoveHudUp(int idx);                    // HUDのレイヤーを1つ上げる
    void MoveHudDown(int idx);                  // HUDのレイヤーを1つ下げる
    void ReFlashHudObjects();                   // HUDパラメーター読み込み
    void SetPreviewMode(bool enabled);
    bool IsPreviewMode() const { return bPreviewMode; }

    // UIデータ群
    std::vector<CHud*> vHudUI;                  // HUDオブジェクトの配列
    std::vector<OBJ::Transform> vTf;            // HUDの位置・サイズ・回転情報の配列
    std::vector<DirectX::XMFLOAT4> vCol;        // HUDの色情報の配列
    std::vector<std::string> vHudNames;         // HUDのテクスチャ名の配列
    std::vector<std::string> vTextureKeys;      // テクスチャキーの配列
    std::vector<bool> vVisible;                 // 表示するかどうか
    std::vector<float> vDisplayTime;            // 表示している時間
    std::vector<double> vShowStartTime;         // 表示開始時間
    std::vector<float> vDisplayDelay;           // 表示遅延時間
private:
    // HUD情報を格納する構造体
    struct HudInfo
    {
        size_t layer_index;
        std::string texName;
        OBJ::Transform tf;
        DirectX::XMFLOAT4 col;
        bool visible;
        float displayTime;
        float displayDelay;
    };

    //HUD情報保管用 (プレビューモード時に使用)
    struct HudBackup 
    {
        std::string textureName;
        OBJ::Transform tf;
        DirectX::XMFLOAT4 col;
        bool visible;
        float displayTime;
    };

    std::vector<HudBackup> vBackupHud;
    static const char* DefaultKeys[3]; //デフォルトキー
    bool bPreviewMode = false;  // プレビューモードフラグ

};