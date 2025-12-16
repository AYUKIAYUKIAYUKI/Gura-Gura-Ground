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
#include <bomb.h>
#include <player.h>

using json = nlohmann::json;

int ObstacleEditer::m_CurrentParamIndex = 0;
float ObstacleEditer::s_LoadedSpawnX = 0.0f, ObstacleEditer::s_LoadedSpawnY = 0.0f, ObstacleEditer::s_LoadedSpawnZ = 0.0f;
float ObstacleEditer::s_LoadedSpeedX = 0.0f, ObstacleEditer::s_LoadedSpeedY = 0.0f, ObstacleEditer::s_LoadedSpeedZ = 0.0f;
int ObstacleEditer::s_SpawnTimePresetCount = ObstacleEditer::SPAWN_PRESET_MAX;
std::vector<float> ObstacleEditer::s_AssignedSpawnTimes(ObstacleEditer::PARAM_SET_MAX, 5.0f);
std::vector<float> ObstacleEditer::s_SpawnTimePresets(ObstacleEditer::SPAWN_PRESET_MAX, 5.0f);
std::vector<std::pair<int, int>> ObstacleEditer::s_AssignedSpawnParamIndices = {};
std::vector<ObstacleEditer::ObstacleParam> ObstacleEditer::m_ParamSets(ObstacleEditer::PARAM_SET_MAX);

std::vector<bool> ObstacleEditer::s_SpawnedFlags = {};

static const char* s_ObstacleTypeNames[] = { "Ball", "Bar", "Bomb" };

//============================================================================
// 障害物パラメーター編集処理
//============================================================================
void ObstacleEditer::EditCommonParams()
{
    auto& paramSet = RefParam();

    // 新規追加ボタン
    if (ImGui::Button("Add Obstacle")) 
    {
        paramSet.subParams.push_back(SubObstacleParam{}); // デフォルト値で追加
    }

    // 障害物(subParams)の一覧UI
    for (size_t i = 0; i < paramSet.subParams.size(); ++i) 
    {
        ImGui::Separator();
        ImGui::PushID(static_cast<int>(i));

        SubObstacleParam& obs = paramSet.subParams[i];

        ImGui::Text("Obstacle %zu", i + 1);

        // タイプ
        ImGui::Combo("Type", &obs.ManualObstacleType, s_ObstacleTypeNames, IM_ARRAYSIZE(s_ObstacleTypeNames));

        // 座標
        ImGui::DragFloat("Spawn Pos X", &obs.ObstacleSpawnX, 0.1f, -100.0f, 100.0f);
        ImGui::DragFloat("Spawn Pos Y", &obs.ObstacleSpawnY, 0.1f, 5.0f, 100.0f);
        ImGui::DragFloat("Spawn Pos Z", &obs.ObstacleSpawnZ, 0.1f, -100.0f, 100.0f);

        // 速度
        ImGui::DragFloat("Speed X", &obs.ObstacleSpeedX, 0.1f, -20.0f, 20.0f);
        ImGui::DragFloat("Speed Y", &obs.ObstacleSpeedY, 0.1f, -20.0f, 20.0f);
        ImGui::DragFloat("Speed Z", &obs.ObstacleSpeedZ, 0.1f, -20.0f, 20.0f);
        ImGui::DragFloat("Collider Width", &obs.ColliderWidth, 0.1f, 0.1f, 100.0f);
        ImGui::DragFloat("Collider Height", &obs.ColliderHeight, 0.1f, 0.1f, 100.0f);
        ImGui::DragFloat("Collider Depth", &obs.ColliderDepth, 0.1f, 0.1f, 100.0f);

        if (obs.ManualObstacleType == 2) 
        {
            ImGui::DragInt("Bomb Timer", &obs.BombTimer, 1.0f, 1, 1000);
        }

        // 個別削除ボタン
        if (ImGui::Button("Remove")) 
        {
            paramSet.subParams.erase(paramSet.subParams.begin() + i);
            ImGui::PopID();
            break;
        }

        ImGui::PopID();
    }
}

//============================================================================
// 障害物テストスポーン処理
//============================================================================
void ObstacleEditer::EditerMenu()
{
    useful::MIS::MyImGuiShortcut_BeginWindow("Obstacle Settings");
    const char* paramSetLabels[PARAM_SET_MAX] = { "Param 1", "Param 2", "Param 3", "Param 4", "Param 5" };
    ImGui::Combo("Param Set", &m_CurrentParamIndex, paramSetLabels, PARAM_SET_MAX);

    // 選択中パラメータセットのパラメータを表示・編集
    EditCommonParams();

    bool lastPlayMode = m_PlayMode;
    ImGui::Checkbox("Play Mode", &m_PlayMode);
    if (!m_PlayMode && lastPlayMode) 
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
        ImGui::Text("PresetCount"); ImGui::SameLine();

        if (ImGui::Button("-##PresetCount"))
        {
            if (s_SpawnTimePresetCount > 1)
            {
                s_SpawnTimePresetCount--;
            }
        }

        ImGui::SameLine();
        ImGui::Text("%d", s_SpawnTimePresetCount); ImGui::SameLine();

        if (ImGui::Button("+##PresetCount"))
        {
            if (s_SpawnTimePresetCount < SPAWN_PRESET_MAX)
            {
                s_SpawnTimePresetCount++;
            }
        }

        for (int i = 0; i < s_SpawnTimePresetCount; ++i)
        {
            char label[32];
            snprintf(label, sizeof(label), "SpawnTime %d", i + 1);
            ImGui::DragFloat(label, &s_SpawnTimePresets[i], 0.1f, 0.0f, 100.0f);
        }

        if (ImGui::Button("Assign Random SpawnTimes"))
        {
            AssignRandomSpawnTimes();
        }

        for (int i = 0; i < s_SpawnTimePresetCount; ++i)
        {
            if (i < s_AssignedSpawnParamIndices.size())
            {
                int paramSetIndex = s_AssignedSpawnParamIndices[i].first;
                int subParamIndex = s_AssignedSpawnParamIndices[i].second;
                if (paramSetIndex < (int)m_ParamSets.size() && subParamIndex < (int)m_ParamSets[paramSetIndex].subParams.size()) {
                    const auto& param = m_ParamSets[paramSetIndex].subParams[subParamIndex];
                    ImGui::Text("Time %d : %.2f (Set %d, Item %d, Type:%s Pos: %.1f %.1f %.1f)",
                        i,
                        s_AssignedSpawnTimes[i],
                        paramSetIndex + 1,
                        subParamIndex + 1,
                        (param.ManualObstacleType == 0 ? "Ball" : "Bar"),
                        param.ObstacleSpawnX, param.ObstacleSpawnY, param.ObstacleSpawnZ
                    );
                }
            }
        }
    }
    ImGui::End();
}

//============================================================================
//プレイモード中の自動スポーン処理
//============================================================================
void ObstacleEditer::PlayModeSpawn(float deltaTime)
{
    if (m_PlayMode)
    {
        m_PlayModeElapsedTime += deltaTime;
        for (int i = 0; i < s_SpawnTimePresetCount; ++i)
        {
            int paramSetIdx = s_AssignedSpawnParamIndices[i].first;
            float assignedSpawnTime = s_AssignedSpawnTimes[i];

            if (!s_SpawnedFlags[i] && m_PlayModeElapsedTime >= assignedSpawnTime)
            {
                auto& paramSet = m_ParamSets[paramSetIdx];
                for (size_t subIdx = 0; subIdx < paramSet.subParams.size(); ++subIdx)
                {
                    const auto& sub = paramSet.subParams[subIdx];

                    switch (sub.ManualObstacleType)
                    {
                    case 0: // Ball
                        CObjectManager::CreateRaw<CBall>(
                            [sub, subIdx, paramSetIdx](CBall* p) -> bool
                            {
                                p->SetParamSetIndex(paramSetIdx);
                                p->SetSubParamIndex(static_cast<int>(subIdx));
                                p->FactoryCollider(sub.ColliderWidth, sub.ColliderHeight, sub.ColliderDepth);
                                const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(p->GetCollider());
                                OBJ::Transform TF = {};
                                TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                                p->SetDirection({ sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });
                                pRigidBody->SetWorldTransform(TF);
                                return true;
                            }, OBJ::TYPE::OBSTACLE);
                        break;
                    case 1: // Bar
                        CObjectManager::CreateRaw<CBar>(
                            [sub, subIdx, paramSetIdx](CBar* p) -> bool
                            {
                                p->SetParamSetIndex(paramSetIdx);
                                p->SetSubParamIndex(static_cast<int>(subIdx));
                                p->FactoryCollider(sub.ColliderWidth, sub.ColliderHeight, sub.ColliderDepth);
                                const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(p->GetCollider());
                                OBJ::Transform TF = {};
                                TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                                CBar::SetRotate(TF, { sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });
                                pRigidBody->SetWorldTransform(TF);
                                p->SetDirection({ sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });
                                return true;
                            }, OBJ::TYPE::OBSTACLE);
                        break;
                    case 2: // Bomb
                        CObjectManager::CreateRaw<CBomb>(
                            [sub, subIdx, paramSetIdx](CBomb* p) -> bool
                            {
                                p->SetParamSetIndex(paramSetIdx);
                                p->SetSubParamIndex(static_cast<int>(subIdx));
                                p->FactoryCollider(sub.ColliderWidth, sub.ColliderHeight, sub.ColliderDepth);
                                OBJ::Transform TF = {};
                                TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                                p->SetTransform(TF);
                                p->SetTimer(sub.BombTimer); // タイマー値セット
                                return true;
                            }, OBJ::TYPE::OBSTACLE);
                        break;
                }
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
    m_PlayModeElapsedTime = 0.0f;
    for (auto& paramSet : m_ParamSets) 
    {
        for (auto& sub : paramSet.subParams) 
        {
            sub.Spawned = false;
        }
    }
    AssignRandomSpawnTimes(); // 割当し直す
}

//============================================================================
// 手動スポーン処理
//============================================================================
void ObstacleEditer::TryManualSpawn()
{
    const auto& paramSet = RefParam();
    int thisSetIdx = m_CurrentParamIndex;  // 現在のパラメータセット番号を保持

    for (size_t subIdx = 0; subIdx < paramSet.subParams.size(); ++subIdx)
    {
        const auto& sub = paramSet.subParams[subIdx];
        switch (sub.ManualObstacleType)
        {
        case 0: // Ball
            CObjectManager::CreateRaw<CBall>(
                [sub, subIdx, thisSetIdx](CBall* p) -> bool
                {
                    p->SetParamSetIndex(thisSetIdx);
                    p->SetSubParamIndex((int)subIdx);
                    p->FactoryCollider(sub.ColliderWidth, sub.ColliderHeight, sub.ColliderDepth);
                    const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(p->GetCollider());
                    OBJ::Transform TF = {};
                    TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                    p->SetDirection({ sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });
                    pRigidBody->SetWorldTransform(TF);
                    return true;
                },
                OBJ::TYPE::OBSTACLE);
            break;
        case 1: // Bar
            CObjectManager::CreateRaw<CBar>(
                [sub, subIdx, thisSetIdx](CBar* p) -> bool
                {
                    p->SetParamSetIndex(thisSetIdx);
                    p->SetSubParamIndex((int)subIdx);
                    p->FactoryCollider(sub.ColliderWidth, sub.ColliderHeight, sub.ColliderDepth);
                    const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(p->GetCollider());
                    OBJ::Transform TF = {};
                    TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                    CBar::SetRotate(TF, { sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });
                    pRigidBody->SetWorldTransform(TF);
                    p->SetDirection({ sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });
                    return true;
                },
                OBJ::TYPE::OBSTACLE);
            break;
        case 2: // Bomb
            CObjectManager::CreateRaw<CBomb>(
                [sub, subIdx, thisSetIdx](CBomb* p) -> bool
                {
                    p->SetParamSetIndex(thisSetIdx);
                    p->SetSubParamIndex(static_cast<int>(subIdx));
                    p->FactoryCollider(sub.ColliderWidth, sub.ColliderHeight, sub.ColliderDepth);
                    OBJ::Transform TF = {};
                    TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                    p->SetTransform(TF);
                    p->SetTimer(sub.BombTimer); // タイマー値セット
                    return true;
                }, OBJ::TYPE::OBSTACLE);
            break;
        }
    }
}

//============================================================================
// 障害物パラメーター保存処理
//============================================================================
void ObstacleEditer::SaveParams(const std::string& fileName)
{
    nlohmann::json jsRoot;
    jsRoot["param_sets"] = nlohmann::json::array();

    for (const auto& paramSet : m_ParamSets)
    {
        nlohmann::json jParamSet;
        jParamSet["sub_params"] = nlohmann::json::array();

        for (const auto& sub : paramSet.subParams)
        {
            nlohmann::json jSub;
            jSub["spawnX"] = sub.ObstacleSpawnX;
            jSub["spawnY"] = sub.ObstacleSpawnY;
            jSub["spawnZ"] = sub.ObstacleSpawnZ;
            jSub["speedX"] = sub.ObstacleSpeedX;
            jSub["speedY"] = sub.ObstacleSpeedY;
            jSub["speedZ"] = sub.ObstacleSpeedZ;
            jSub["collider_width"] = sub.ColliderWidth;
            jSub["collider_height"] = sub.ColliderHeight;
            jSub["collider_depth"] = sub.ColliderDepth;
            jSub["manual_type"] = sub.ManualObstacleType;
            jSub["bomb_timer"] = sub.BombTimer;
            jParamSet["sub_params"].push_back(jSub);
        }

        jsRoot["param_sets"].push_back(jParamSet);
    }

    // 生成時間プリセットやプレイモード関連の保存
    jsRoot["spawn_time_presets"] = nlohmann::json::array();
    for (float t : s_SpawnTimePresets)
    {
        jsRoot["spawn_time_presets"].push_back(t);
    }

    jsRoot["spawn_enable_time"] = 3.0f;
    jsRoot["preset_count"] = s_SpawnTimePresetCount;

    std::ofstream ofs(fileName);
    ofs << jsRoot.dump(4);
    ofs.close();
}


//============================================================================
// 障害物パラメーターロード処理
//============================================================================
void ObstacleEditer::LoadParams(const std::string& fileName)
{
    //各種変数の初期化
    m_CurrentParamIndex = 0;
    m_PlayMode = false;
    m_PlayModeElapsedTime = 0.0f;

    m_ParamSets.clear();
    m_ParamSets.resize(PARAM_SET_MAX);

    std::ifstream ifs(fileName);
    if (!ifs) return;
    nlohmann::json jsRoot;
    ifs >> jsRoot;

    // param_setsを読みこむ
    if (jsRoot.contains("param_sets") && jsRoot["param_sets"].is_array())
    {
        for (size_t i = 0; i < m_ParamSets.size(); ++i)
        {
            m_ParamSets[i].subParams.clear();
            if (i < jsRoot["param_sets"].size())
            {
                const auto& jParamSet = jsRoot["param_sets"][i];
                if (jParamSet.contains("sub_params") && jParamSet["sub_params"].is_array())
                {
                    for (const auto& jSub : jParamSet["sub_params"])
                    {
                        SubObstacleParam sub;
                        sub.ObstacleSpawnX = jSub.value("spawnX", 0.0f);
                        sub.ObstacleSpawnY = jSub.value("spawnY", 10.0f);
                        sub.ObstacleSpawnZ = jSub.value("spawnZ", 15.0f);
                        sub.ObstacleSpeedX = jSub.value("speedX", 0.0f);
                        sub.ObstacleSpeedY = jSub.value("speedY", 0.0f);
                        sub.ObstacleSpeedZ = jSub.value("speedZ", -5.0f);
                        sub.ColliderWidth = jSub.value("collider_width", 3.0f);
                        sub.ColliderHeight = jSub.value("collider_height", 3.0f);
                        sub.ColliderDepth = jSub.value("collider_depth", 3.0f);
                        sub.ManualObstacleType = jSub.value("manual_type", 0);
                        sub.BombTimer = jSub.value("bomb_timer", 300);
                        sub.Spawned = false;
                        m_ParamSets[i].subParams.push_back(sub);
                    }
                }
            }
        }
    }

    // プリセット数/生成時間
    s_SpawnTimePresetCount = jsRoot.value("preset_count", s_SpawnTimePresetCount);

    if (jsRoot.contains("spawn_time_presets") && jsRoot["spawn_time_presets"].is_array())
    {
        int arrSize = jsRoot["spawn_time_presets"].size();
        for (int i = 0; i < arrSize && i < SPAWN_PRESET_MAX; ++i)
        {
            s_SpawnTimePresets[i] = jsRoot["spawn_time_presets"][i].get<float>();
        }
    }
    AssignRandomSpawnTimes();
}

//============================================================================
// 出現時間プリセットの抽選処理
//============================================================================
void ObstacleEditer::AssignRandomSpawnTimes()
{
    // 各割当配列をプリセット数でリサイズする
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

    // すべての(ParamSetIdx, subParamIdx)ペアをリスト化
    std::vector<std::pair<int, int>> allPairs;
    for (int paramSetIdx = 0; paramSetIdx < (int)m_ParamSets.size(); ++paramSetIdx)
    {
        const auto& paramSet = m_ParamSets[paramSetIdx];
        for (int subParamIdx = 0; subParamIdx < (int)paramSet.subParams.size(); ++subParamIdx)
        {
            allPairs.emplace_back(paramSetIdx, subParamIdx);
        }
    }

    // 出現候補が無い場合は何もしない
    if (allPairs.empty())
    {
        return;
    }

    // 順番をランダムシャッフル
    std::random_device randomDevice;
    std::mt19937 rngEngine(randomDevice());
    std::shuffle(allPairs.begin(), allPairs.end(), rngEngine);

    // プリセット数分だけ負荷
    for (int presetIndex = 0; presetIndex < s_SpawnTimePresetCount; ++presetIndex)
    {
        const auto& paramIndexPair = allPairs[presetIndex % allPairs.size()];
        s_AssignedSpawnParamIndices[presetIndex] = paramIndexPair;
        s_AssignedSpawnTimes[presetIndex] = s_SpawnTimePresets[presetIndex % s_SpawnTimePresets.size()];
        s_SpawnedFlags[presetIndex] = false;
    }
}