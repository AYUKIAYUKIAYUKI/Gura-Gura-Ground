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
    struct ObstacleParam {
        float ObstacleSpeedX = 0.0f;
        float ObstacleSpeedY = 0.0f;
        float ObstacleSpeedZ = -5.0f;
        float ObstacleSpawnX = 0.0f;
        float ObstacleSpawnY = 10.0f;
        float ObstacleSpawnZ = 15.0f;
        int  ManualObstacleType = 0;
    };

    static const int PARAM_SET_MAX = 5;
    static std::vector<ObstacleParam> s_ParamSets;
    static int s_CurrentParamIndex;
    static ObstacleParam& RefParam() { return s_ParamSets[s_CurrentParamIndex]; }

    static void EditCommonParams();
    static void ShowEditerMenu();
    static void TryManualSpawn();
    static void TryAutoSpawn(float gameTime);
    static void SaveParams(const std::string& fileName);    // パラメーターの保存
    static void LoadParams(const std::string& fileName);    // パラメーターの読み込み
    static void ApplyLoadedParams(float gameTime);          // 時間で障害物を出現させる

    static bool IsAutoSpawnEnabled() { return s_AutoSpawnEnabled; }
    static float GetAutoSpawnInterval() { return s_ObstacleSpawnInterval; }
    static int GetAutoSpawnObstacleType() { return s_AutoSpawnObstacleType; }

private:
    static float        s_ObstacleLastSpawnTime;
    static bool         s_AutoSpawnEnabled;
    static float        s_ObstacleSpawnInterval;
    static int          s_AutoSpawnObstacleType;
    static int          s_ManualObstacleType;
    static bool         s_LoadedParamsValid;
    static float        s_LoadedSpawnEnableTime;
    static bool         s_LoadedShown; // 一度出現したかどうか
    static float        s_LoadedSpawnX, s_LoadedSpawnY, s_LoadedSpawnZ;
    static float        s_LoadedSpeedX, s_LoadedSpeedY, s_LoadedSpeedZ;
    static int          s_LoadedType; // 0:Ball, 1:Bar
};