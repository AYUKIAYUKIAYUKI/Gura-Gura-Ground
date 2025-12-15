//============================================================================
// 
// 障害物エディター [obstacle_editer.cpp]
// Author : Sohta Kuki
// 
//============================================================================

#include "obstacle_editer.h"
#include "ball.h"
#include "bar.h"
#include "API.object.manager.h"
#include <API.rigidbody.h>
#include <random>

using json = nlohmann::json;

int ObstacleEditer::s_LoadedType = 0; // 0:Ball, 1:Bar
int ObstacleEditer::s_CurrentParamIndex = 0;
bool ObstacleEditer::s_PlayMode = false; //プレイモードかどうか
bool ObstacleEditer::s_LoadedParamsValid = false;
bool  ObstacleEditer::s_LoadedShown = false;
float ObstacleEditer::s_LoadedSpawnX = 0.0f, ObstacleEditer::s_LoadedSpawnY = 0.0f, ObstacleEditer::s_LoadedSpawnZ = 0.0f;
float ObstacleEditer::s_LoadedSpeedX = 0.0f, ObstacleEditer::s_LoadedSpeedY = 0.0f, ObstacleEditer::s_LoadedSpeedZ = 0.0f;
float ObstacleEditer::s_PlayModeElapsedTime = 0.0f;
int ObstacleEditer::s_SpawnTimePresetCount = ObstacleEditer::SPAWN_PRESET_MAX;
std::vector<float> ObstacleEditer::s_AssignedSpawnTimes(ObstacleEditer::PARAM_SET_MAX, 5.0f);
std::vector<float> ObstacleEditer::s_SpawnTimePresets(ObstacleEditer::SPAWN_PRESET_MAX, 5.0f);
float ObstacleEditer::s_ObstacleLastSpawnTime = 0.0f;
std::vector<int> ObstacleEditer::s_AssignedSpawnParamIndices = {};

std::vector<ObstacleEditer::ObstacleParam> ObstacleEditer::s_ParamSets(ObstacleEditer::PARAM_SET_MAX);
std::vector<bool> ObstacleEditer::s_SpawnedFlags = {};

static const char* s_ObstacleTypeNames[] = { "Ball", "Bar" };

//============================================================================
// 障害物パラメーター編集処理
//============================================================================
void ObstacleEditer::EditCommonParams()
{
    auto& param = RefParam();
    ImGui::Text("Obstacle Param");
    ImGui::Combo("Obstacle Type", &param.ManualObstacleType, s_ObstacleTypeNames, IM_ARRAYSIZE(s_ObstacleTypeNames));
    ImGui::DragFloat("Spawn Pos X", &param.ObstacleSpawnX, 0.1f, -100.0f, 100.0f);
    ImGui::DragFloat("Spawn Pos Y", &param.ObstacleSpawnY, 0.1f, 5.0f, 100.0f);
    ImGui::DragFloat("Spawn Pos Z", &param.ObstacleSpawnZ, 0.1f, -100.0f, 100.0f);
    ImGui::DragFloat("Spawm Speed X", &param.ObstacleSpeedX, 0.1f, -20.0f, 20.0f);
    ImGui::DragFloat("Spawm Speed Y", &param.ObstacleSpeedY, 0.1f, -20.0f, 20.0f);
    ImGui::DragFloat("Spawm Speed Z", &param.ObstacleSpeedZ, 0.1f, -20.0f, 20.0f);
}

//============================================================================
// 障害物テストスポーン処理
//============================================================================
void ObstacleEditer::EditerMenu()
{
    useful::MIS::MyImGuiShortcut_BeginWindow("Obstacle Settings");
    const char* paramSetLabels[PARAM_SET_MAX] = { "Param 1", "Param 2", "Param 3", "Param 4", "Param 5" };
    ImGui::Combo("Param Set", &s_CurrentParamIndex, paramSetLabels, PARAM_SET_MAX);

    // 選択中パラメータセットのパラメータを表示・編集
    EditCommonParams();

    bool lastPlayMode = s_PlayMode;
    ImGui::Checkbox("Play Mode", &s_PlayMode);
    if (!s_PlayMode && lastPlayMode) 
    {
        ResetPlayMode();
    }

    ImGui::Separator();

    if (ImGui::Button("Spawn Obstacle"))
    {
        TryManualSpawn();
    }

    if (ImGui::Button("Save Param"))
    {
        SaveParams("Data\\JSON\\obscale_table.json");
    }

    ImGui::End();
}

//============================================================================
// 各出現パラメーターを抽選して障害物を出現させる処理
//============================================================================
void ObstacleEditer::SpawnTimePresetEditor()
{
    if (ImGui::Begin("SpawnTime Preset Editor"))
    {
        //プリセット数の編集
        ImGui::Text("PresetCount"); ImGui::SameLine();

        //プリセット数を1以上に制限して減少
        if (ImGui::Button("-##PresetCount"))
        {
            if (s_SpawnTimePresetCount > 1)
                s_SpawnTimePresetCount--;
        }

        ImGui::SameLine();
        ImGui::Text("%d", s_SpawnTimePresetCount); ImGui::SameLine();

        //プリセット数を最大値以内で増加
        if (ImGui::Button("+##PresetCount"))
        {
            if (s_SpawnTimePresetCount < SPAWN_PRESET_MAX)
                s_SpawnTimePresetCount++;
        }

        // 追加分の出現時間を1.0fで初期化させる
        if ((int)s_SpawnTimePresets.size() < s_SpawnTimePresetCount)
        {
            s_SpawnTimePresets.resize(s_SpawnTimePresetCount, 1.0f);
        }

        // 余分な要素を切り詰める
        else if ((int)s_SpawnTimePresets.size() > s_SpawnTimePresetCount)
        {
            s_SpawnTimePresets.resize(s_SpawnTimePresetCount);
        }

        // 出現する時間を編集
        for (int i = 0; i < s_SpawnTimePresetCount; ++i)
        {
            char label[32];
            snprintf(label, sizeof(label), "SpawmTime %d", i + 1);
            ImGui::DragFloat(label, &s_SpawnTimePresets[i], 0.1f, 0.0f, 100.0f);
        }

        //出現パラメーターのランダム抽選
        if (ImGui::Button("Assign Random SpawnTimes"))
        {
            AssignRandomSpawnTimes();
        }

        //各パラメーターの表示
        for (int i = 0; i < s_SpawnTimePresetCount; ++i)
        {
            int paramIdx = s_AssignedSpawnParamIndices.size() > i ? s_AssignedSpawnParamIndices[i] : 0;
            const auto& param = s_ParamSets[paramIdx];
            ImGui::Text("Time %d : %.2f (Param %d, Type:%s Pos: %.1f %.1f %.1f)",
                i, s_AssignedSpawnTimes.size() > i ? s_AssignedSpawnTimes[i] : 0.0f,
                paramIdx + 1,
                (param.ManualObstacleType == 0 ? "Ball" : "Bar"),
                param.ObstacleSpawnX, param.ObstacleSpawnY, param.ObstacleSpawnZ);
        }
    }
    ImGui::End();
}


//============================================================================
//プレイモード中の自動スポーン処理
//============================================================================
void ObstacleEditer::PlayModeSpawn(float deltaTime)
{
    if (s_PlayMode)
    {
        s_PlayModeElapsedTime += deltaTime;
        for (int i = 0; i < s_SpawnTimePresetCount; ++i)
        {
            int paramIdx = s_AssignedSpawnParamIndices[i];
            auto& param = s_ParamSets[paramIdx];
            float assignedSpawnTime = s_AssignedSpawnTimes[i];
            if (!s_SpawnedFlags[i] && s_PlayModeElapsedTime >= assignedSpawnTime)
            {
                switch (param.ManualObstacleType)
                {
                case 0: // Ball
                    CObject::Create<CBall>(
                        [param](CBall* p) -> bool
                        {
                            p->FactoryCollider(3.0f, 3.0f, 3.0f);

                            const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(p->GetCollider());

                            OBJ::Transform TF = {};
                            TF.Pos = { param.ObstacleSpawnX, param.ObstacleSpawnY, param.ObstacleSpawnZ };
                            p->SetDirection({ param.ObstacleSpeedX, param.ObstacleSpeedY, param.ObstacleSpeedZ });

                            pRigidBody->SetWorldTransform(TF);

                            return true;
                        }, OBJ::TYPE::OBSTACLE);

                    break;
                case 1: // Bar
                    CObject::Create<CBar>(
                        [param](CBar* p) -> bool
                        {
                            p->FactoryCollider(1.5f, 15.0f, 1.5f);
                            const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(p->GetCollider());
                            OBJ::Transform TF = {};
                            TF.Pos = { param.ObstacleSpawnX, param.ObstacleSpawnY, param.ObstacleSpawnZ };
                            CBar::SetRotate(TF, { param.ObstacleSpeedX, param.ObstacleSpeedY, param.ObstacleSpeedZ });
                            pRigidBody->SetWorldTransform(TF);
                            p->SetDirection({ param.ObstacleSpeedX, param.ObstacleSpeedY, param.ObstacleSpeedZ });
                            return true;
                        }, OBJ::TYPE::OBSTACLE);
                    break;
                }
                s_SpawnedFlags[i] = true;
            }
        }
    }
}


//============================================================================
// プレイモード中の経過時間リセット＆スポーンフラグをリセット
//============================================================================
void ObstacleEditer::ResetPlayMode()
{
    s_PlayModeElapsedTime = 0.0f;
    for (auto& param : s_ParamSets) param.Spawned = false;

    AssignRandomSpawnTimes();
}

//============================================================================
// 手動スポーン処理
//============================================================================
void ObstacleEditer::TryManualSpawn()
{
    const auto& param = RefParam();
    int type = param.ManualObstacleType;
    switch (type)
    {
    case 0: // Ball
        CObject::Create<CBall>(
            [](CBall* p) -> bool
            {
                p->FactoryCollider(3.0f, 3.0f, 3.0f);
                return true;
            },
            OBJ::TYPE::OBSTACLE);
        break;
    case 1: // Bar
        CObject::Create<CBar>(
            [](CBar* p) -> bool
            {
                p->FactoryCollider(1.5f, 15.0f, 1.5f);
                return true;
            },
            OBJ::TYPE::OBSTACLE);
        break;
    }
}

//============================================================================
// 障害物パラメーター保存処理
//============================================================================
void ObstacleEditer::SaveParams(const std::string& fileName)
{
    json js;

    // 各パラメータセットを配列で保存
    js["param_sets"] = json::array();
    for (int i = 0; i < PARAM_SET_MAX; ++i)
    {
        const auto& param = s_ParamSets[i];
        json jp;
        jp["spawnX"] = param.ObstacleSpawnX;
        jp["spawnY"] = param.ObstacleSpawnY;
        jp["spawnZ"] = param.ObstacleSpawnZ;
        jp["speedX"] = param.ObstacleSpeedX;
        jp["speedY"] = param.ObstacleSpeedY;
        jp["speedZ"] = param.ObstacleSpeedZ;
        jp["manual_type"] = param.ManualObstacleType;
        js["param_sets"].push_back(jp);
    }

    js["spawn_time_presets"] = json::array();
    for (int i = 0; i < s_SpawnTimePresetCount; ++i) // 最大値まで保存
    {
        js["spawn_time_presets"].push_back(s_SpawnTimePresets[i]);
    }

    js["spawn_enable_time"] = 3.0f;
    js["preset_count"] = s_SpawnTimePresetCount;

    std::ofstream ofs(fileName);
    ofs << js.dump(4);
    ofs.close();
}


//============================================================================
// 障害物パラメーターロード処理
//============================================================================
void ObstacleEditer::LoadParams(const std::string& fileName)
{
    std::ifstream ifs(fileName);
    if (!ifs) return;
    json js;
    ifs >> js;

    // 最大保存数だけ読みこみ
    if (js.contains("param_sets") && js["param_sets"].is_array()) 
    {
        for (int i = 0; i < PARAM_SET_MAX; ++i) 
        {
            if (i < js["param_sets"].size()) 
            {
                auto& param = s_ParamSets[i];
                auto jp = js["param_sets"][i];
                param.ObstacleSpawnX = jp.value("spawnX", 0.0f);
                param.ObstacleSpawnY = jp.value("spawnY", 10.0f);
                param.ObstacleSpawnZ = jp.value("spawnZ", 15.0f);
                param.ObstacleSpeedX = jp.value("speedX", 0.0f);
                param.ObstacleSpeedY = jp.value("speedY", 0.0f);
                param.ObstacleSpeedZ = jp.value("speedZ", -5.0f);
                param.ManualObstacleType = jp.value("manual_type", 0);
                param.Spawned = false;
            }
        }
    }

    if (js.contains("preset_count")) {
        int preset_count = js["preset_count"].get<int>();
        if (preset_count >= 1 && preset_count <= SPAWN_PRESET_MAX) {
            s_SpawnTimePresetCount = preset_count;
        }
    }
    else {
        s_SpawnTimePresetCount = SPAWN_PRESET_MAX;
    }

    if (js.contains("spawn_time_presets") && js["spawn_time_presets"].is_array())
    {
        int arrSize = js["spawn_time_presets"].size();
        for (int i = 0; i < arrSize && i < SPAWN_PRESET_MAX; ++i)
        {
            s_SpawnTimePresets[i] = js["spawn_time_presets"][i].get<float>();
        }
    }
    AssignRandomSpawnTimes(); // 読み込み後にランダムで割り当てる
}

//============================================================================
// 出現時間プリセットの抽選処理
//============================================================================
void ObstacleEditer::AssignRandomSpawnTimes()
{
    // ベクトルのサイズ調整
    if ((int)s_SpawnedFlags.size() != s_SpawnTimePresetCount)
    {
        s_SpawnedFlags.resize(s_SpawnTimePresetCount);
    }
    if ((int)s_AssignedSpawnTimes.size() != s_SpawnTimePresetCount)
    {
        s_AssignedSpawnTimes.resize(s_SpawnTimePresetCount);
    }
    if ((int)s_AssignedSpawnParamIndices.size() != s_SpawnTimePresetCount)
    {
        s_AssignedSpawnParamIndices.resize(s_SpawnTimePresetCount);
    }

    // フラグ初期化
    for (int i = 0; i < s_SpawnTimePresetCount; ++i)
    {
        s_SpawnedFlags[i] = false;
    }

    // 割り当て順をランダムにシャッフル
    std::vector<int> indices(s_SpawnTimePresetCount);
    for (int i = 0; i < s_SpawnTimePresetCount; ++i)
    {
        indices[i] = i;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    std::uniform_int_distribution<> distParam(0, PARAM_SET_MAX - 1);

    // プリセット値をランダムな順序でs_AssignedSpawnTimesへ格納
    for (int i = 0; i < s_SpawnTimePresetCount; ++i)
    {
        s_AssignedSpawnTimes[i] = s_SpawnTimePresets[indices[i]];
        s_AssignedSpawnParamIndices[i] = distParam(g);
    }
}