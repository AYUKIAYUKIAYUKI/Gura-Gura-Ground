�ｿ//============================================================================
// 
// 髫懷ｮｳ迚ｩ繧ｨ繝�ぅ繧ｿ繝ｼ [obstacle_editer.h]
// Author : Sohta Kuki
// 
//============================================================================

#pragma once

#include "debuff_behavior.h"

//****************************************************
// 髫懷ｮｳ迚ｩ繧ｨ繝�ぅ繧ｿ繝ｼ繧ｯ繝ｩ繧ｹ縺ｮ螳夂ｾｩ
//****************************************************
class ObstacleEditer
{
public:
    //========================
    // 髫懷ｮｳ迚ｩ繝代Λ繝｡繝ｼ繧ｿ繧ｻ繝�ヨ
    //========================
    enum class OBS_TYPE : unsigned char
    {
        NONE = 0,  // 辟｡縺
        BALL,      // 繝懊�繝ｫ
        BAR,       // 譽
        BOMB,      // 繝懊Β
        TORNADO,   // 遶懷ｷｻ
        FALLTETRA, // 荳企Κ關ｽ荳狗黄
        PENDULUM,  // 謖ｯ繧雁ｭ
        BOOMERANG, // 繝悶�繝｡繝ｩ繝ｳ
        BIRDSTRIKE, // 魑･縺ｮ鄒､繧
        BARREL,    //繧ｿ繝ｫ+繧ｪ繧､繝ｫ
        MAX
    };

    struct SubObstacleParam {
        OBS_TYPE ManualObstacleType = OBS_TYPE::NONE;  // 髫懷ｮｳ迚ｩ繧ｿ繧､繝(0:B
        float ObstacleSpeedX = 0.0f;                   // 蛻晞溷ｺｦX
        float ObstacleSpeedY = 0.0f;                   // 蛻晞溷ｺｦY
        float ObstacleSpeedZ = -5.0f;                  // 蛻晞溷ｺｦZ
        float ObstacleSpawnX = 0.0f;                   // 逕滓�菴咲ｽｮX
        float ObstacleSpawnY = 10.0f;                  // 逕滓�菴咲ｽｮY
        float ObstacleSpawnZ = 10.0f;                  // 逕滓�菴咲ｽｮZ
        float ColliderWidth = 3.0f;                    // 繧ｳ繝ｩ繧､繝繝ｼ讓ｪ繧ｵ繧､繧ｺ
        float ColliderHeight = 3.0f;                   // 繧ｳ繝ｩ繧､繝繝ｼ邵ｦ繧ｵ繧､繧ｺ
        float ColliderDepth = 3.0f;                    // 繧ｳ繝ｩ繧､繝繝ｼ繧ｵ繧､繧ｺ豺ｱ縺
        int BombTimer = 300;                           // 繧ｿ繧､繝槭�蛟､(繝懊Β逕ｨ)
        float BoomerangOmega = 1.0f;   // 騾溷ｺｦ
        float BoomerangRadius = 12.0f; // 蜊雁ｾ
        float BoomerangBasePower = 20.0f;      // 蝓ｺ譛ｬ蜷ｹ縺｣鬟帙�繝代Ρ繝ｼ
        float BoomerangAddBySpeed = 80.0f;     // 騾溷ｺｦ萓晏ｭ伜刈邂
        float BoomerangMaxFinalPower = 350.0f; //譛螟ｧ
        int   BoomerangHitCooldown = 10;
        int BoomerangMovePattern = 0; //繝悶�繝｡繝ｩ繝ｳ縺ｮ遘ｻ蜍
    };

    struct ObstacleParam {
        std::vector<SubObstacleParam> subParams; // 隍�焚髫懷ｮｳ迚ｩ諠�ｱ繧偵∪縺ｨ繧√ｋ
    };

    struct DebuffConfig {
        float DecayValue = 0.3f;
        float InertiaValue = 1.0f;
    };
    static DebuffConfig s_StampConfig, s_BirdConfig, s_OilConfig;

    static const int PARAM_SET_MAX = 50;                // パラメータセット最大数
    static const int SPAWN_PRESET_MAX = 50;

    void EditCommonParams();                    // 蜈ｱ騾壹ヱ繝ｩ繝｡繝ｼ繧ｿ邱ｨ髮�I
    void EditerMenu();                          // 繝｡繧､繝ｳ邱ｨ髮�え繧｣繝ｳ繝峨え
    void SpawnTimePresetEditor();               // 繧ｹ繝昴�繝ｳ繧ｿ繧､繝繝励Μ繧ｻ繝�ヨ邱ｨ髮
    void TryManualSpawn();                      // 謇句虚逕滓�
    void SaveParams(const std::string& fileName);    // 繝代Λ繝｡繝ｼ繧ｿ菫晏ｭ
    void LoadParams(const std::string& fileName);    // 繝代Λ繝｡繝ｼ繧ｿ隱ｭ縺ｿ霎ｼ縺ｿ
    void SaveCurrentSubObstacleParam(const std::string& fileName);   // 迴ｾ蝨ｨ縺ｮ繝代Λ繝｡繝ｼ繧ｿ繧ｻ繝�ヨ縺縺台ｿ晏ｭ
    void LoadCurrentSubObstacleParam(const std::string& fileName);   // 荳願ｨ倥�繝代Λ繝｡繝ｼ繧ｿ繧ｻ繝�ヨ縺ｮ隱ｭ縺ｿ霎ｼ縺ｿ
    void AssignRandomSpawnTimes();              // 繧ｹ繝昴�繝ｳ繧ｿ繧､繝蜑ｲ繧雁ｽ薙※�医Λ繝ｳ繝繝蛹厄ｼ
    void PlayModeSpawn(float deltaTime);        // 繝励Ξ繧､繝｢繝ｼ繝峨�閾ｪ蜍慕函謌仙�逅
    void ResetPlayMode();                       // 繝励Ξ繧､繝｢繝ｼ繝峨Μ繧ｻ繝�ヨ
    void ShowGlobalGimmickSettingsWindow();

    bool m_PlayMode;                        // 繝励Ξ繧､繝｢繝ｼ繝峨ヵ繝ｩ繧ｰ
    float m_PlayModeElapsedTime;             // 繝励Ξ繧､繝｢繝ｼ繝臥ｵ碁℃譎る俣
    static int m_CurrentParamIndex;                    // 迴ｾ蝨ｨ邱ｨ髮�ｸｭ繧､繝ｳ繝�ャ繧ｯ繧ｹ
    static std::vector<ObstacleParam> m_ParamSets;     // 繝代Λ繝｡繝ｼ繧ｿ繧ｻ繝�ヨ驟榊�
    static ObstacleParam& RefParam() { return m_ParamSets[m_CurrentParamIndex]; }
    static float s_DecayValue; //荳企Κ關ｽ荳矩囿螳ｳ迚ｩ縺ｫ謗･隗ｦ縺励◆譎ゅ�貂幃溷､
private:
    float m_ObstacleTimerElapsedTime = 0.0f;
    bool m_CustomTimerNeedReset = false;
    float m_CustomTimerResetCountdown = 0.0f;

    static int s_ParamSetCount;
    static void ChangeParamSetCount(int delta);

    static int m_selectedSubParamIndex;
    static int s_SpawnTimePresetCount;
    static float s_LoadedSpawnX, s_LoadedSpawnY, s_LoadedSpawnZ; // 蜃ｺ迴ｾ菴咲ｽｮ
    static float s_LoadedSpeedX, s_LoadedSpeedY, s_LoadedSpeedZ; // 蜃ｺ迴ｾ騾溷ｺｦ
    static std::vector<float> s_SpawnTimePresets;
    static std::vector<float> s_AssignedSpawnTimes;
    static std::vector<std::pair<int, int>> s_AssignedSpawnParamIndices; // ParamSet縺ｮindex縺ｨsubParam縺ｮindex
    static std::vector<bool> s_SpawnedFlags;
    static std::vector<int> s_SpawnPlayerThresholds;
    static std::vector<int> s_ForcedParamSetIndices;

};