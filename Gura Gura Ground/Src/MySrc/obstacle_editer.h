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
        float ColliderWidth = 3.0f;
        float ColliderHeight = 3.0f;
        float ColliderDepth = 3.0f;
        int BombTimer = 300;                           // タイマー値(ボム用)
    };

    struct ObstacleParam {
        std::vector<SubObstacleParam> subParams; // 複数障害物情報をまとめる
    };

    static const int PARAM_SET_MAX = 5;                // パラメータセット最大数
    static const int SPAWN_PRESET_MAX = 10;

    void EditCommonParams();                    // 共通パラメータ編集UI
    void EditerMenu();                          // メイン編集ウィンドウ
    void SpawnTimePresetEditor();               // スポーンタイムプリセット編集
    void TryManualSpawn();                      // 手動生成
    void SaveParams(const std::string& fileName);    // パラメータ保存
    void LoadParams(const std::string& fileName);    // パラメータ読み込み
    void AssignRandomSpawnTimes();              // スポーンタイム割り当て（ランダム化）
    void PlayModeSpawn(float deltaTime);        // プレイモード・自動生成処理
    void ResetPlayMode();                       // プレイモードリセット

    bool m_PlayMode;                        // プレイモードフラグ
    float m_PlayModeElapsedTime;             // プレイモード経過時間
    static int m_CurrentParamIndex;                    // 現在編集中インデックス
    static std::vector<ObstacleParam> m_ParamSets;     // パラメータセット配列
    static ObstacleParam& RefParam() { return m_ParamSets[m_CurrentParamIndex]; }
private:
    static int s_SpawnTimePresetCount;
    static float s_LoadedSpawnX, s_LoadedSpawnY, s_LoadedSpawnZ; // 直近ロード位置
    static float s_LoadedSpeedX, s_LoadedSpeedY, s_LoadedSpeedZ; // 直近ロード速度
    static std::vector<float> s_SpawnTimePresets;
    static std::vector<float> s_AssignedSpawnTimes;
    static std::vector<std::pair<int, int>> s_AssignedSpawnParamIndices; // (ParamSetのindexとsubParamのindex)
    static std::vector<bool> s_SpawnedFlags;
};