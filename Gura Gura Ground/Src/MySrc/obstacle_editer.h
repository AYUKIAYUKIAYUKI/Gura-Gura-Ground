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
    struct ObstacleParam {
        int  ManualObstacleType = 0;                   // 障害物タイプ(0:Ball, 1:Bar)
        bool Spawned = false;                          // 生成済みフラグ
        float ObstacleSpeedX = 0.0f;                   // 初速度X
        float ObstacleSpeedY = 0.0f;                   // 初速度Y
        float ObstacleSpeedZ = -5.0f;                  // 初速度Z
        float ObstacleSpawnX = 0.0f;                   // 生成位置X
        float ObstacleSpawnY = 10.0f;                  // 生成位置Y
        float ObstacleSpawnZ = 15.0f;                  // 生成位置Z
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

    static std::vector<ObstacleParam> s_ParamSets;     // パラメータセット配列
    static int s_CurrentParamIndex;                    // 現在編集中インデックス
    static ObstacleParam& RefParam() { return s_ParamSets[s_CurrentParamIndex]; }
    static bool     s_PlayMode;                        // プレイモードフラグ
    static float    s_PlayModeElapsedTime;             // プレイモード経過時間

private:
    static int          s_LoadedType;                  // 今ロードした障害物タイプ（0:Ball, 1:Bar）
    static bool         s_LoadedParamsValid;           // ロードしたパラメータが有効か
    static bool         s_LoadedShown;                 // 一度表示済みか
    static float        s_LoadedSpawnX, s_LoadedSpawnY, s_LoadedSpawnZ; // 直近ロード位置
    static float        s_LoadedSpeedX, s_LoadedSpeedY, s_LoadedSpeedZ; // 直近ロード速度
    static float        s_ObstacleLastSpawnTime;       // 最後に生成した時刻
    static std::vector<float> s_SpawnTimePresets;
    static std::vector<float> s_AssignedSpawnTimes;
    static std::vector<int> s_AssignedSpawnParamIndices;
    static std::vector<bool> s_SpawnedFlags;


    static int s_SpawnTimePresetCount;
};