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

using json = nlohmann::json;

float ObstacleEditer::s_ObstacleLastSpawnTime = 0.0f;
bool  ObstacleEditer::s_AutoSpawnEnabled = true;
float ObstacleEditer::s_ObstacleSpawnInterval = 3.0f;
int   ObstacleEditer::s_AutoSpawnObstacleType = 0;

bool ObstacleEditer::s_LoadedParamsValid = false;
float ObstacleEditer::s_LoadedSpawnEnableTime = 0.0f;
bool  ObstacleEditer::s_LoadedShown = false;
float ObstacleEditer::s_LoadedSpawnX = 0.0f, ObstacleEditer::s_LoadedSpawnY = 0.0f, ObstacleEditer::s_LoadedSpawnZ = 0.0f;
float ObstacleEditer::s_LoadedSpeedX = 0.0f, ObstacleEditer::s_LoadedSpeedY = 0.0f, ObstacleEditer::s_LoadedSpeedZ = 0.0f;
int   ObstacleEditer::s_LoadedType = 0; // 0:Ball, 1:Bar

std::vector<ObstacleEditer::ObstacleParam> ObstacleEditer::s_ParamSets(ObstacleEditer::PARAM_SET_MAX);
int ObstacleEditer::s_CurrentParamIndex = 0;

static const char* s_ObstacleTypeNames[] = { "Ball", "Bar" };

//============================================================================
// 障害物パラメーター編集処理
//============================================================================
void ObstacleEditer::EditCommonParams()
{
    auto& param = RefParam();
    ImGui::Text("Obstacle Param");
    ImGui::DragFloat("Spawn Pos X", &param.ObstacleSpawnX, 0.1f, -100.0f, 100.0f);
    ImGui::DragFloat("Spawn Pos Y", &param.ObstacleSpawnY, 0.1f, 5.0f, 100.0f);
    ImGui::DragFloat("Spawn Pos Z", &param.ObstacleSpawnZ, 0.1f, -100.0f, 100.0f);
    ImGui::DragFloat("Spawm Speed X", &param.ObstacleSpeedX, 0.1f, -20.0f, 20.0f);
    ImGui::DragFloat("Spawm Speed Y", &param.ObstacleSpeedY, 0.1f, -20.0f, 20.0f);
    ImGui::DragFloat("Spawm Speed Z", &param.ObstacleSpeedZ, 0.1f, -20.0f, 20.0f);
    ImGui::Combo("Manual Obstacle Type (Param)", &param.ManualObstacleType, s_ObstacleTypeNames, IM_ARRAYSIZE(s_ObstacleTypeNames));
}

//============================================================================
// 障害物テストスポーン処理
//============================================================================
void ObstacleEditer::ShowEditerMenu()
{
    useful::MIS::MyImGuiShortcut_BeginWindow("Obstacle Settings");
    const char* paramSetLabels[PARAM_SET_MAX] = { "Param 1", "Param 2", "Param 3", "Param 4", "Param 5" };
    ImGui::Combo("Param Set", &s_CurrentParamIndex, paramSetLabels, PARAM_SET_MAX);

    // 選択中パラメータセットのパラメータを表示・編集
    EditCommonParams();

    ImGui::Checkbox("Enable Auto Spawn", &s_AutoSpawnEnabled);
    ImGui::DragFloat("Auto Spawn Interval", &s_ObstacleSpawnInterval, 0.1f, 0.1f, 30.0f);

    ImGui::Separator();

    ImGui::Combo("Auto Obstacle Type", &s_AutoSpawnObstacleType, s_ObstacleTypeNames, IM_ARRAYSIZE(s_ObstacleTypeNames));
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
// 自動スポーン処理
//============================================================================
void ObstacleEditer::TryAutoSpawn(float gameTime)
{
    if (!s_AutoSpawnEnabled) return;
    if (gameTime - s_ObstacleLastSpawnTime >= s_ObstacleSpawnInterval)
    {
        switch (s_AutoSpawnObstacleType)
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
        s_ObstacleLastSpawnTime = gameTime;
    }
}

//============================================================================
// 障害物パラメーター保存処理
//============================================================================
void ObstacleEditer::SaveParams(const std::string& fileName)
{
    json js;

    js["enable"] = s_AutoSpawnEnabled;
    js["interval"] = s_ObstacleSpawnInterval;
    js["auto_type"] = s_AutoSpawnObstacleType;

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

    js["spawn_enable_time"] = 3.0f;

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
    s_AutoSpawnEnabled = js.value("enable", true);
    s_ObstacleSpawnInterval = js.value("interval", 3.0f);
    s_AutoSpawnObstacleType = js.value("auto_type", 0);

    // 最大保存数だけ読みこみ
    if (js.contains("param_sets") && js["param_sets"].is_array()) {
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
            }
        }
    }

    s_LoadedSpawnEnableTime = js.value("spawn_enable_time", 3.0f);
}

//============================================================================
// 時間で障害物を出現させる処理
//============================================================================
void ObstacleEditer::ApplyLoadedParams(float gameTime)
{
    if (!s_LoadedParamsValid || s_LoadedShown) return;

    // ロード時のパラメータで出現させる
    if (gameTime > s_LoadedSpawnEnableTime)
    {
        switch (s_LoadedType)
        {
        case 0: // Ball
            CObject::Create<CBall>( [](CBall* p) -> bool{return true;},OBJ::TYPE::OBSTACLE);
            break;
        case 1: // Bar
            CObject::Create<CBar>([](CBar* p) -> bool { return true;}, OBJ::TYPE::OBSTACLE);
            break;
        }
        s_LoadedShown = true;
        s_LoadedParamsValid = false;
    }
}