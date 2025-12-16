//============================================================================
// 
// 障害物エディター [obstacle_editer.h]
// Author : Sohta Kuki
// 
//============================================================================

#pragma once

//****************************************************
// 障害物エディタークラスの定義
//****************************************************
class ObstacleEditer
{
public:
    //========================
    // 障害物パラメータセット
    //========================
    struct SubObstacleParam {
        int  ManualObstacleType = 0;                   // 障害物タイプ(0:Ball, 1:Bar)
        bool Spawned = false;                          // 生成済みフラグ
        float ObstacleSpeedX = 0.0f;                   // 初速度X
        float ObstacleSpeedY = 0.0f;                   // 初速度Y
        float ObstacleSpeedZ = -5.0f;                  // 初速度Z
        float ObstacleSpawnX = 0.0f;                   // 生成位置X
        float ObstacleSpawnY = 10.0f;                  // 生成位置Y
        float ObstacleSpawnZ = 15.0f;                  // 生成位置Z
    };

    struct ObstacleParam {
        std::vector<SubObstacleParam> subParams; // 複数障害物情報をまとめる
    };

    static const int PARAM_SET_MAX = 5;                // パラメータセット最大数
    static const int SPAWN_PRESET_MAX = 10;

    static void EditCommonParams();                    // 共通パラメータ編集UI
    static void EditerMenu();                          // メイン編集ウィンドウ
    static void SpawnTimePresetEditor();               // スポーンタイムプリセット編集
    static void TryManualSpawn();                      // 手動生成
    static void SaveParams(const std::string& fileName);    // パラメータ保存
    static void LoadParams(const std::string& fileName);    // パラメータ読み込み
    static void AssignRandomSpawnTimes();              // スポーンタイム割り当て（ランダム化）
    static void PlayModeSpawn(float deltaTime);        // プレイモード・自動生成処理
    static void ResetPlayMode();                       // プレイモードリセット

    static int s_CurrentParamIndex;                    // 現在編集中インデックス
    static bool s_PlayMode;                        // プレイモードフラグ
    static float s_PlayModeElapsedTime;             // プレイモード経過時間
    static std::vector<ObstacleParam> s_ParamSets;     // パラメータセット配列
    static ObstacleParam& RefParam() { return s_ParamSets[s_CurrentParamIndex]; }

private:
    static int s_SpawnTimePresetCount;
    static float s_LoadedSpawnX, s_LoadedSpawnY, s_LoadedSpawnZ; // 直近ロード位置
    static float s_LoadedSpeedX, s_LoadedSpeedY, s_LoadedSpeedZ; // 直近ロード速度
    static std::vector<float> s_SpawnTimePresets;
    static std::vector<float> s_AssignedSpawnTimes;
    static std::vector<std::pair<int, int>> s_AssignedSpawnParamIndices; // (ParamSetのindexとsubParamのindex)
    static std::vector<bool> s_SpawnedFlags;
};