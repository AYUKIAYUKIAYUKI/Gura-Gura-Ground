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
    enum class OBS_TYPE : unsigned char
    {
        NONE = 0,  // 無し
        BALL,      // ボール
        BAR,       // 棒
        BOMB,      // ボム
        TORNADO,   // 竜巻
        FALLTETRA, // 上部落下物
        PENDULUM,  // 振り子
        BOOMERANG, // ブーメラン
        BIRDSTRIKE, // 鳥の群れ
        BARREL,    //タル+オイル
        MAX
    };

    struct SubObstacleParam {
        OBS_TYPE ManualObstacleType = OBS_TYPE::NONE;  // 障害物タイプ(0:B
        float ObstacleSpeedX = 0.0f;                   // 初速度X
        float ObstacleSpeedY = 0.0f;                   // 初速度Y
        float ObstacleSpeedZ = -5.0f;                  // 初速度Z
        float ObstacleSpawnX = 0.0f;                   // 生成位置X
        float ObstacleSpawnY = 10.0f;                  // 生成位置Y
        float ObstacleSpawnZ = 10.0f;                  // 生成位置Z
        float ColliderWidth = 3.0f;                    // コライダー横サイズ
        float ColliderHeight = 3.0f;                   // コライダー縦サイズ
        float ColliderDepth = 3.0f;                    // コライダーサイズ深さ
        int BombTimer = 300;                           // タイマー値(ボム用)
        float BoomerangOmega = 1.0f;   // 速度
        float BoomerangRadius = 12.0f; // 半径
        float BoomerangBasePower = 20.0f;      // 基本吹っ飛びパワー
        float BoomerangAddBySpeed = 80.0f;     // 速度依存加算
        float BoomerangMaxFinalPower = 350.0f; //最大
        int   BoomerangHitCooldown = 10;
        int BoomerangMovePattern = 0; //ブーメランの移動
    };

    struct ObstacleParam {
        std::vector<SubObstacleParam> subParams; // 複数障害物情報をまとめる
    };

    static const int PARAM_SET_MAX = 5;                // パラメータセット最大数
    static const int SPAWN_PRESET_MAX = 50;

    void EditCommonParams();                    // 共通パラメータ編集UI
    void EditerMenu();                          // メイン編集ウィンドウ
    void SpawnTimePresetEditor();               // スポーンタイムプリセット編集
    void TryManualSpawn();                      // 手動生成
    void SaveParams(const std::string& fileName);    // パラメータ保存
    void LoadParams(const std::string& fileName);    // パラメータ読み込み
    void AssignRandomSpawnTimes();              // スポーンタイム割り当て（ランダム化）
    void PlayModeSpawn(float deltaTime);        // プレイモード・自動生成処理
    void ResetPlayMode();                       // プレイモードリセット
    void ShowGlobalGimmickSettingsWindow();

    bool m_PlayMode;                        // プレイモードフラグ
    float m_PlayModeElapsedTime;             // プレイモード経過時間
    static int m_CurrentParamIndex;                    // 現在編集中インデックス
    static std::vector<ObstacleParam> m_ParamSets;     // パラメータセット配列
    static ObstacleParam& RefParam() { return m_ParamSets[m_CurrentParamIndex]; }
    static float s_DecayValue; //上部落下障害物に接触した時の減速値
private:
    static int s_SpawnTimePresetCount;
    static float s_LoadedSpawnX, s_LoadedSpawnY, s_LoadedSpawnZ; // 出現位置
    static float s_LoadedSpeedX, s_LoadedSpeedY, s_LoadedSpeedZ; // 出現速度
    static std::vector<float> s_SpawnTimePresets;
    static std::vector<float> s_AssignedSpawnTimes;
    static std::vector<std::pair<int, int>> s_AssignedSpawnParamIndices; // ParamSetのindexとsubParamのindex
    static std::vector<bool> s_SpawnedFlags;
    static std::vector<int> s_SpawnPlayerThresholds;
    static std::vector<int> s_ForcedParamSetIndices;
};