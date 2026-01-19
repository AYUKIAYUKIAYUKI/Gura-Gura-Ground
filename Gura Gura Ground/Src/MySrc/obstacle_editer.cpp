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
#include <tornado.h>
#include <FallTetra.h>
#include <pendulum.h>
#include <boomerang.h>
#include <birdstrike.h>
#include "Barrel.h"

using json = nlohmann::json;

int ObstacleEditer::m_CurrentParamIndex = 0;
float ObstacleEditer::s_LoadedSpawnX = 0.0f, ObstacleEditer::s_LoadedSpawnY = 0.0f, ObstacleEditer::s_LoadedSpawnZ = 0.0f;
float ObstacleEditer::s_LoadedSpeedX = 0.0f, ObstacleEditer::s_LoadedSpeedY = 0.0f, ObstacleEditer::s_LoadedSpeedZ = 0.0f;
int ObstacleEditer::s_SpawnTimePresetCount = ObstacleEditer::SPAWN_PRESET_MAX;
std::vector<float> ObstacleEditer::s_AssignedSpawnTimes(ObstacleEditer::PARAM_SET_MAX, 5.0f);
std::vector<float> ObstacleEditer::s_SpawnTimePresets(ObstacleEditer::SPAWN_PRESET_MAX, 5.0f);
std::vector<std::pair<int, int>> ObstacleEditer::s_AssignedSpawnParamIndices = {};
std::vector<ObstacleEditer::ObstacleParam> ObstacleEditer::m_ParamSets(ObstacleEditer::PARAM_SET_MAX);
std::vector<int> ObstacleEditer::s_SpawnPlayerThresholds(ObstacleEditer::SPAWN_PRESET_MAX, 4);
std::vector<int> ObstacleEditer::s_ForcedParamSetIndices(ObstacleEditer::SPAWN_PRESET_MAX, 0);
float ObstacleEditer::s_DecayValue = 0.3f;

std::vector<bool> ObstacleEditer::s_SpawnedFlags = {};

//============================================================================
// 障害物パラメーター編集処理
//============================================================================
void ObstacleEditer::EditCommonParams()
{
    auto& paramSet = RefParam();

    static int selectedSubParamIndex = 0; 

    static SubObstacleParam s_CopiedSubParam;
    static bool s_ParamCopied = false;

    // 障害物を追加
    if (ImGui::Button(reinterpret_cast<const char*>(u8"障害物を追加")))
    {
        paramSet.subParams.push_back(SubObstacleParam{});
    }

    auto& playerList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);
    for (auto& playerObj : playerList) 
    {
        auto player = std::dynamic_pointer_cast<CPlayer>(playerObj);
        if (!player) continue;
        auto fallTetra = player->GetFallTetraBehavior();
        if (fallTetra) {
            // FallTetra_Behaviorのm_DecayValueへ値を渡すsetter関数
            fallTetra->SetDecayValue(s_DecayValue);
        }
    }

    if (selectedSubParamIndex >= 0 && selectedSubParamIndex < (int)paramSet.subParams.size())
    {
        // コピー
        if (ImGui::Button(reinterpret_cast<const char*>(u8"編集中の障害物パラメーターをコピー")))
        {
            s_CopiedSubParam = paramSet.subParams[selectedSubParamIndex];
            s_ParamCopied = true;
        }
        ImGui::SameLine();
        // ペースト
        bool canPaste = s_ParamCopied &&
            (selectedSubParamIndex >= 0 && selectedSubParamIndex < (int)paramSet.subParams.size());
        ImGui::BeginDisabled(!canPaste);
        if (ImGui::Button(reinterpret_cast<const char*>(u8"コピーしたパラメーターをペースト"))) {
            if (canPaste) {
                // ManualObstacleTypeを除きすべてのパラメータをペースト
                auto& dst = paramSet.subParams[selectedSubParamIndex];
                OBS_TYPE prevType = dst.ManualObstacleType;
                int prevPattern = dst.BoomerangMovePattern;

                dst = s_CopiedSubParam;
                // ManualObstacleTypeとインデックス（型）は貼り替え先のまま維持
                dst.ManualObstacleType = prevType;
                dst.BoomerangMovePattern = prevPattern;
            }
        }
        ImGui::EndDisabled();
    }

    // 障害物(subParams)の一覧UI
    // リスト表示
    ImGui::Text(reinterpret_cast<const char*>(u8"障害物リスト:"));
    for (int i = 0; i < (int)paramSet.subParams.size(); ++i)
    {
        char label[32];
        snprintf(label, sizeof(label), reinterpret_cast<const char*>(u8"障害物[%d]"), i + 1);
        // 選択型リストボタン
        if (ImGui::Selectable(reinterpret_cast<const char*>(label), selectedSubParamIndex == i))
        {
            selectedSubParamIndex = i;
        }
        // 削除ボタン
        char deleteLabel[32];
        snprintf(deleteLabel, sizeof(deleteLabel), reinterpret_cast<const char*>(u8"障害物[%d]を削除##del%d"), i + 1);
        if (ImGui::Button(reinterpret_cast<const char*>(deleteLabel)))
        {
            paramSet.subParams.erase(paramSet.subParams.begin() + i);
            if (selectedSubParamIndex >= i && selectedSubParamIndex > 0)
            {
                selectedSubParamIndex--; // 削除時選択インデックス調整
            }
            if (paramSet.subParams.empty())
            {
                selectedSubParamIndex = -1; // 空になったら未選択
            }
            break;
        }
    }
    ImGui::NewLine();

    // 選択された障害物パラメータ編集
    if (selectedSubParamIndex >= 0 && selectedSubParamIndex < (int)paramSet.subParams.size())
    {
        SubObstacleParam& obs = paramSet.subParams[selectedSubParamIndex];

        ImGui::Separator();
        ImGui::Text(reinterpret_cast<const char*>(u8"障害物パラメーター [%d]"), selectedSubParamIndex + 1);

        // 障害物タイプ
        int currentType = static_cast<int>(obs.ManualObstacleType);
        const char* typeNames[] = 
        {
            reinterpret_cast<const char*>(u8"None"),
            reinterpret_cast<const char*>(u8"ボール"),
            reinterpret_cast<const char*>(u8"バー"),
            reinterpret_cast<const char*>(u8"爆弾"),
            reinterpret_cast<const char*>(u8"竜巻"),
            reinterpret_cast<const char*>(u8"ドッスン"),
            reinterpret_cast<const char*>(u8"振り子"),
            reinterpret_cast<const char*>(u8"ブーメラン"),
            reinterpret_cast<const char*>(u8"鳥の群れ"),
            reinterpret_cast<const char*>(u8"タル+オイル")
        };

        if (ImGui::Combo(reinterpret_cast<const char*>(u8"出現させる障害物"), &currentType, typeNames, static_cast<int>(OBS_TYPE::MAX)))
        {
            obs.ManualObstacleType = static_cast<OBS_TYPE>(currentType); // 整数型から OBS_TYPE型へ再キャストする
        }

        // スポーン座標 (竜巻以外)
        if (obs.ManualObstacleType != OBS_TYPE::TORNADO && obs.ManualObstacleType != OBS_TYPE::BIRDSTRIKE)
        {
            ImGui::DragFloat(reinterpret_cast<const char*>(u8"スポーン座標 X"), &obs.ObstacleSpawnX, 0.1f, -100.0f, 100.0f);

            if (obs.ManualObstacleType == OBS_TYPE::FALLTETRA)
            {
                ImGui::Text(reinterpret_cast<const char*>(u8"スポーン座標 Yは15.0fで固定に設定されています"));
            }

            if (obs.ManualObstacleType == OBS_TYPE::PENDULUM)
            {
                ImGui::Text(reinterpret_cast<const char*>(u8"スポーン座標 Yは20.0fで固定に設定されています"));
            }

            if (obs.ManualObstacleType == OBS_TYPE::BOOMERANG)
            {
                ImGui::Text(reinterpret_cast<const char*>(u8"スポーン座標 Yは9.0fで固定に設定されています"));
            }

            //スポーン座標 Y (ドッスン以外)
            if (obs.ManualObstacleType != OBS_TYPE::FALLTETRA && obs.ManualObstacleType != OBS_TYPE::PENDULUM && obs.ManualObstacleType != OBS_TYPE::BOOMERANG)
            {
                ImGui::DragFloat(reinterpret_cast<const char*>(u8"スポーン座標 Y"), &obs.ObstacleSpawnY, 0.1f, 5.0f, 100.0f);
            }

            ImGui::DragFloat(reinterpret_cast<const char*>(u8"スポーン座標 Z"), &obs.ObstacleSpawnZ, 0.1f, -100.0f, 100.0f);
        }

        // 移動速度 (ドッスンと竜巻、振り子、ブーメラン、鳥の群れ以外)
        if (obs.ManualObstacleType != OBS_TYPE::FALLTETRA && obs.ManualObstacleType != OBS_TYPE::TORNADO && obs.ManualObstacleType != OBS_TYPE::BIRDSTRIKE &&
            obs.ManualObstacleType != OBS_TYPE::PENDULUM && obs.ManualObstacleType != OBS_TYPE::BOMB && obs.ManualObstacleType != OBS_TYPE::BOOMERANG)
        {
            ImGui::DragFloat(reinterpret_cast<const char*>(u8"移動速度 X"), &obs.ObstacleSpeedX, 0.1f, -20.0f, 20.0f);
            ImGui::DragFloat(reinterpret_cast<const char*>(u8"移動速度 Y"), &obs.ObstacleSpeedY, 0.1f, -20.0f, 20.0f);
            ImGui::DragFloat(reinterpret_cast<const char*>(u8"移動速度 Z"), &obs.ObstacleSpeedZ, 0.1f, -20.0f, 20.0f);
        }

        if (obs.ManualObstacleType == OBS_TYPE::BOOMERANG)
        {
            ImGui::Separator();
            ImGui::Text(reinterpret_cast<const char*>(u8"ブーメラン詳細パラメータ"));

            const char* movePatternNames[4] = { reinterpret_cast < const char*>(u8"奥から手前"), reinterpret_cast <const char*>(u8"手前から奥"), reinterpret_cast <const char*>(u8"右から左"), reinterpret_cast <const char*>(u8"左から右") };
            ImGui::Combo(reinterpret_cast<const char*>(u8"ブーメランの移動"), &obs.BoomerangMovePattern, movePatternNames, 4);

            ImGui::DragFloat(reinterpret_cast<const char*>(u8"移動速度"), &obs.BoomerangOmega, 0.01f, 0.1f, 5.0f);
            ImGui::DragFloat(reinterpret_cast<const char*>(u8"半径"), &obs.BoomerangRadius, 0.1f, 5.0f, 50.0f);
            ImGui::DragFloat(reinterpret_cast<const char*>(u8"吹っ飛び力"), &obs.BoomerangBasePower, 1.0f, 0.0f, 500.0f);
            ImGui::DragFloat(reinterpret_cast<const char*>(u8"速度依存加算"), &obs.BoomerangAddBySpeed, 1.0f, 0.0f, 500.0f);
            ImGui::DragFloat(reinterpret_cast<const char*>(u8"最大吹っ飛び力"), &obs.BoomerangMaxFinalPower, 1.0f, 0.0f, 1000.0f);
            ImGui::DragInt(reinterpret_cast<const char*>(u8"ヒット後のクールタイム"), &obs.BoomerangHitCooldown, 1, 1, 100);
        }

        ImGui::DragFloat(reinterpret_cast<const char*>(u8"コライダーの幅"), &obs.ColliderWidth, 0.1f, 0.1f, 100.0f);
        ImGui::DragFloat(reinterpret_cast<const char*>(u8"コライダーの高さ"), &obs.ColliderHeight, 0.1f, 0.1f, 100.0f);
        ImGui::DragFloat(reinterpret_cast<const char*>(u8"コライダーの深度"), &obs.ColliderDepth, 0.1f, 0.1f, 100.0f);

        // ボム固有パラメータ入力
        if (obs.ManualObstacleType == OBS_TYPE::BOMB)
        {
            ImGui::DragInt(reinterpret_cast<const char*>(u8"爆発までの時間"), &obs.BombTimer, 1.0f, 1, 1000);
        }
    }
    else
    {
        ImGui::Text(reinterpret_cast<const char*>(u8"障害物を選択してください。"));
    }
}

//============================================================================
// 障害物テストスポーン処理
//============================================================================
void ObstacleEditer::EditerMenu()
{
    useful::MIS::MyImGuiShortcut_BeginWindow(reinterpret_cast<const char*>(u8"障害物設定メニュー"));

    bool lastPlayMode = m_PlayMode;
    ImGui::Checkbox(reinterpret_cast<const char*>(u8"プレイモード"), &m_PlayMode);

    // プレイモードに入るときに、割当て未実行分があれば再抽選させる
    if (m_PlayMode && !lastPlayMode)
    {
        bool bAssign = false;
        if (s_AssignedSpawnParamIndices.size() != (size_t)s_SpawnTimePresetCount)
        {
            bAssign = true;
        }
        else 
        {
            for (const auto& idx : s_AssignedSpawnParamIndices)
            {
                if (idx.first < 0 || idx.second < 0) 
                {
                    bAssign = true;
                    break;
                }
            }
        }
        if (bAssign) 
        {
            AssignRandomSpawnTimes();
        }
    }

    if (!m_PlayMode && lastPlayMode)
    {
        ResetPlayMode();
    }

    if (ImGui::Button(reinterpret_cast<const char*>(u8"選択中のプリセットを出現")))
    {
        TryManualSpawn();
    }

    if (ImGui::Button(reinterpret_cast<const char*>(u8"全てのプリセットを保存")))
    {
        SaveParams("Data\\JSON\\obscale_table.json");
    }

    const char* paramSetLabels[PARAM_SET_MAX] = { "Preset 1", "Preset 2", "Preset 3", "Preset 4", "Preset 5" };
    ImGui::Combo(reinterpret_cast<const char*>(u8"編集するプリセット"), &m_CurrentParamIndex, paramSetLabels, PARAM_SET_MAX);

    // 選択中パラメータセットのパラメータを表示・編集
    EditCommonParams();

    //ギミック全体効果のメニュー
    ShowGlobalGimmickSettingsWindow();

    ImGui::End();
}

//============================================================================
// 各出現パラメーターを抽選して障害物を出現させる処理
//============================================================================
void ObstacleEditer::SpawnTimePresetEditor()
{
    if (ImGui::Begin(reinterpret_cast<const char*>(u8"障害物出現タイミング編集")))
    {
        ImGui::Text(reinterpret_cast<const char*>(u8"ゲームタイム %.2f 秒"), m_PlayModeElapsedTime);

        // 残りプレイヤー数を取得して表示
        const auto& playerList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);
        int remainingPlayers = static_cast<int>(playerList.size());
        ImGui::Text(reinterpret_cast<const char*>(u8"残りプレイヤー数 [%d 人]"), remainingPlayers);

        if (m_PlayMode == false)
        {
            ImGui::Text(reinterpret_cast<const char*>(u8"出現回数編集")); ImGui::SameLine();

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
        }

        //出現時間プリセットの数が変更されたら出現時間の分もリサイズする
        if ((int)s_AssignedSpawnTimes.size() != s_SpawnTimePresetCount)
        {
            s_AssignedSpawnTimes.resize(s_SpawnTimePresetCount, 5.0f);
        }

        if ((int)s_SpawnPlayerThresholds.size() != s_SpawnTimePresetCount)
        {
            s_SpawnPlayerThresholds.resize(s_SpawnTimePresetCount, 4);
        }

        if ((int)s_ForcedParamSetIndices.size() != s_SpawnTimePresetCount)
        {
            s_ForcedParamSetIndices.resize(s_SpawnTimePresetCount, 0);
        }

        if (ImGui::Button(reinterpret_cast<const char*>(u8"設定を適用＆シャッフル抽選する")))
        {
            AssignRandomSpawnTimes();
        }

        ImGui::Separator();

        for (int i = 0; i < s_SpawnTimePresetCount; ++i)
        {
            char label[32];
            snprintf(label, sizeof(label), reinterpret_cast<const char*>(u8"出現時間 [%d]"), i + 1);
            ImGui::DragFloat(label, &s_SpawnTimePresets[i], 0.1f, 0.0f, 100.0f);

            char minusBtn[32], plusBtn[32];
            snprintf(minusBtn, sizeof(minusBtn), "-##force%d", i);
            snprintf(plusBtn, sizeof(plusBtn), "+##force%d", i);


            if (ImGui::Button(minusBtn)) {
                if (s_ForcedParamSetIndices[i] > 0)
                    s_ForcedParamSetIndices[i]--;
            }
            ImGui::SameLine();
            ImGui::Text("%d", s_ForcedParamSetIndices[i]);
            ImGui::SameLine();
            if (ImGui::Button(plusBtn)) {
                if (s_ForcedParamSetIndices[i] < PARAM_SET_MAX)
                    s_ForcedParamSetIndices[i]++;
            }
            ImGui::SameLine();
            ImGui::Text(reinterpret_cast<const char*>(u8"0=ランダム出現 1～5=プリセット出現"));

            // 残りプレイヤー数出現条件設定
            if (i >= s_SpawnPlayerThresholds.size()) s_SpawnPlayerThresholds.resize(i + 1, 99);
            snprintf(minusBtn, sizeof(minusBtn), "-##th%d", i);
            snprintf(plusBtn, sizeof(plusBtn), "+##th%d", i);

            if (ImGui::Button(minusBtn))
            {
                if (s_SpawnPlayerThresholds[i] > 1)
                    s_SpawnPlayerThresholds[i]--;
            }

            ImGui::SameLine();
            ImGui::Text("%d", s_SpawnPlayerThresholds[i]);
            ImGui::SameLine();

            if (ImGui::Button(plusBtn))
            {
                if (s_SpawnPlayerThresholds[i] < 4)
                    s_SpawnPlayerThresholds[i]++;
            }
            ImGui::SameLine();
            ImGui::Text(reinterpret_cast<const char*>(u8"人以下のプレイヤー数で出現"));
            ImGui::Separator();
        }

        for (int i = 0; i < s_SpawnTimePresetCount; ++i)
        {
            if (i < s_AssignedSpawnParamIndices.size())
            {
                int paramSetIndex = s_AssignedSpawnParamIndices[i].first;
                int subParamIndex = s_AssignedSpawnParamIndices[i].second;
                if (paramSetIndex < (int)m_ParamSets.size() && subParamIndex < (int)m_ParamSets[paramSetIndex].subParams.size()) {
                    ImGui::Text(reinterpret_cast<const char*>(u8"出現時間 [%d] : %.2f (出現プリセット : Preset %d)"),
                        i + 1,
                        s_AssignedSpawnTimes[i],
                        paramSetIndex + 1
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

        const auto& playerList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);
        int remainingPlayers = static_cast<int>(playerList.size());

        for (int i = 0; i < s_SpawnTimePresetCount; ++i)
        {
            int paramSetIdx = s_AssignedSpawnParamIndices[i].first;
            float assignedSpawnTime = s_AssignedSpawnTimes[i];

            int playerTh = 4;
            if (i < s_SpawnPlayerThresholds.size()) 
            {
                playerTh = s_SpawnPlayerThresholds[i];
            }
            if (!(remainingPlayers <= playerTh)) continue;

            if (!s_SpawnedFlags[i] && m_PlayModeElapsedTime >= assignedSpawnTime)
            {
                auto& paramSet = m_ParamSets[paramSetIdx];
                for (size_t subIdx = 0; subIdx < paramSet.subParams.size(); ++subIdx)
                {
                    const auto& sub = paramSet.subParams[subIdx];

                    //各種障害物の生成
                    switch (sub.ManualObstacleType)
                    {
                    case OBS_TYPE::BALL:
                        CObjectManager::CreateShare<CBall>([sub, subIdx, paramSetIdx](CBall* p) -> bool
                            {
                                p->SetParamSetIndex(paramSetIdx);
                                p->SetSubParamIndex(static_cast<int>(subIdx));
                                p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight /2, sub.ColliderDepth / 2);
                                const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(p->GetCollider());
                                OBJ::Transform TF = {};
                                TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                                p->SetDirection({ sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });
                                pRigidBody->SetWorldTransform(TF);
                                return true;
                            }, OBJ::TYPE::OBSTACLE);
                        break;
                    case OBS_TYPE::BAR:
                        CObjectManager::CreateShare<CBar>([sub, subIdx, paramSetIdx](CBar* p) -> bool
                            {
                                p->SetParamSetIndex(paramSetIdx);
                                p->SetSubParamIndex(static_cast<int>(subIdx));
                                p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight /2, sub.ColliderDepth / 2);
                                const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(p->GetCollider());
                                OBJ::Transform TF = {};
                                TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                                CBar::SetRotate(TF, { sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });
                                pRigidBody->SetWorldTransform(TF);
                                p->SetDirection({ sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });
                                return true;
                            }, OBJ::TYPE::OBSTACLE);
                        break;
                    case OBS_TYPE::BOMB:
                        CObjectManager::CreateShare<CBomb>([sub, subIdx, paramSetIdx](CBomb* p) -> bool
                            {
                                p->SetParamSetIndex(paramSetIdx);
                                p->SetSubParamIndex(static_cast<int>(subIdx));
                                p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight /2, sub.ColliderDepth / 2);
                                OBJ::Transform TF = {};
                                TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                                p->SetTransform(TF);
                                p->SetTimer(sub.BombTimer); // タイマー値セット
                                return true;
                            }, OBJ::TYPE::OBSTACLE);
                        break;
                    case OBS_TYPE::TORNADO:
                        CObjectManager::CreateShare<CTornado>([sub, subIdx, paramSetIdx](CTornado* p) -> bool
                            {
                                const float fSpanField = 15.0f;
                                p->SetParamSetIndex(paramSetIdx);
                                p->SetSubParamIndex(static_cast<int>(subIdx));
                                float Size = 3.0f;
                                float Pos = fSpanField + 5.0f;         // 地面サイズ+オフセットで基準位置
                                OBJ::Transform TF = p->GetTransform();
                                TF.Pos = { -Pos, 0.0f, Pos };
                                p->SetTransform(TF);
                                p->SetStartPos(TF.Pos);
                                p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight /2, sub.ColliderDepth / 2);
                                p->SetDepth(Pos * 2.0f); // 奥行き
                                p->SetWidth(Pos * 2.0f); // 幅
                                return true;
                            }, OBJ::TYPE::OBSTACLE);
                        break;
                    case OBS_TYPE::FALLTETRA:
                        CObjectManager::CreateShare<CFallTetra>([sub, subIdx, paramSetIdx](CFallTetra* p) -> bool
                            {
                                p->SetParamSetIndex(paramSetIdx);
                                p->SetSubParamIndex(static_cast<int>(subIdx));
                                // 幅・高さ・奥行きをセット
                                p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight /2, sub.ColliderDepth / 2);
                                OBJ::Transform TF = {};
                                TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                                p->SetTransform(TF);
                                return true;
                            }, OBJ::TYPE::OBSTACLE);
                        break;
                    case OBS_TYPE::PENDULUM:
                        CObjectManager::CreateShare<CPendulum>([sub, subIdx, paramSetIdx](CPendulum* p) -> bool
                            {
                                p->SetParamSetIndex(paramSetIdx);
                                p->SetSubParamIndex(static_cast<int>(subIdx));
                                p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight /2, sub.ColliderDepth / 2);
                                OBJ::Transform TF = {};
                                TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                                p->SetTransform(TF);
                                return true;
                            }, OBJ::TYPE::OBSTACLE);
                        break;
                    case OBS_TYPE::BOOMERANG:
                        CObjectManager::CreateShare<CBoomerang>([sub, subIdx, paramSetIdx](CBoomerang* p) -> bool {
                            p->SetParamSetIndex(paramSetIdx);
                            p->SetSubParamIndex(static_cast<int>(subIdx));
                            p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight /2, sub.ColliderDepth / 2);
                            p->SetMovePattern(sub.BoomerangMovePattern);
                            OBJ::Transform TF = {};
                            TF.Pos = { sub.ObstacleSpawnX, 9.0f, sub.ObstacleSpawnZ };
                            p->SetTransform(TF);

                            // パラメータセット
                            p->SetBoomerangParams(
                                sub.BoomerangOmega,
                                sub.BoomerangRadius,
                                sub.BoomerangBasePower,
                                sub.BoomerangAddBySpeed,
                                sub.BoomerangMaxFinalPower,
                                sub.BoomerangHitCooldown
                            );
                            return true;
                            }, OBJ::TYPE::OBSTACLE);
                        break;
                    case OBS_TYPE::BIRDSTRIKE:
                        CObjectManager::CreateShare<CBirdStrike>([sub, subIdx, paramSetIdx](CBirdStrike* p) -> bool
                            {
                                p->SetParamSetIndex(paramSetIdx);
                                p->SetSubParamIndex(static_cast<int>(subIdx));
                                p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight /2, sub.ColliderDepth / 2);
                                OBJ::Transform TF = {};
                                TF.Pos = { 0.0f, 0.0f, 0.0f };
                                p->SetTransform(TF);
                                return true;
                            }, OBJ::TYPE::OBSTACLE);
                        break;
                    case OBS_TYPE::BARREL:
                        CObjectManager::CreateShare<CBarrel>([sub, subIdx, paramSetIdx](CBarrel* p) -> bool
                            {
                                p->SetParamSetIndex(paramSetIdx);
                                p->SetSubParamIndex(static_cast<int>(subIdx));
                                // 先にコライダーを生成
                                p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight / 2, sub.ColliderDepth / 2);

                                OBJ::Transform TF = {};
                                TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };

                                // 必要であれば角度設定。速度ベースにしたい場合はここを工夫
                                CBarrel::SetRotate(TF, { sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });
                                p->SetTransform(TF);

                                // 進行方向もエディタ値を使う
                                p->SetDirection({ sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });

                                const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(p->GetCollider());
                                if (pRigidBody) {
                                    pRigidBody->SetWorldTransform(TF);
                                }

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

    // スポーンフラグを初期化させる
    for (int presetIndex = 0; presetIndex < s_SpawnTimePresetCount; ++presetIndex)
    {
        s_SpawnedFlags[presetIndex] = false;
    }
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

        //各種障害物の生成
        switch (sub.ManualObstacleType)
        {
        case OBS_TYPE::BALL:
            CObjectManager::CreateShare<CBall>([sub, subIdx, thisSetIdx](CBall* p) -> bool
                {
                    p->SetParamSetIndex(thisSetIdx);
                    p->SetSubParamIndex((int)subIdx);
                    p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight / 2, sub.ColliderDepth / 2);
                    const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(p->GetCollider());
                    OBJ::Transform TF = {};
                    TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                    p->SetDirection({ sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });
                    pRigidBody->SetWorldTransform(TF);
                    return true;
                },
                OBJ::TYPE::OBSTACLE);
            break;
        case OBS_TYPE::BAR:
            CObjectManager::CreateShare<CBar>([sub, subIdx, thisSetIdx](CBar* p) -> bool
                {
                    p->SetParamSetIndex(thisSetIdx);
                    p->SetSubParamIndex((int)subIdx);
                    p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight / 2, sub.ColliderDepth / 2);
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
        case OBS_TYPE::BOMB:
            CObjectManager::CreateShare<CBomb>([sub, subIdx, thisSetIdx](CBomb* p) -> bool
                {
                    p->SetParamSetIndex(thisSetIdx);
                    p->SetSubParamIndex(static_cast<int>(subIdx));
                    p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight / 2, sub.ColliderDepth / 2);
                    OBJ::Transform TF = {};
                    TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                    p->SetTransform(TF);
                    p->SetTimer(sub.BombTimer); // タイマー値セット
                    return true;
                }, OBJ::TYPE::OBSTACLE);
            break;
        case OBS_TYPE::TORNADO:
            CObjectManager::CreateShare<CTornado>([sub, subIdx, thisSetIdx](CTornado* p) -> bool
                {
                    const float fSpanField = 15.0f;
                    p->SetParamSetIndex(thisSetIdx);
                    p->SetSubParamIndex(static_cast<int>(subIdx));
                    float Size = 3.0f;
                    float Pos = fSpanField + 5.0f;         // 地面サイズ+オフセットで基準位置
                    OBJ::Transform TF = p->GetTransform();
                    TF.Pos = { -Pos, 0.0f, Pos };
                    p->SetTransform(TF);
                    p->SetStartPos(TF.Pos);
                    p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight / 2, sub.ColliderDepth / 2);
                    p->SetDepth(Pos * 2.0f); // 奥行き
                    p->SetWidth(Pos * 2.0f); // 幅
                    return true;
                }, OBJ::TYPE::OBSTACLE);
            break;
        case OBS_TYPE::FALLTETRA:
            CObjectManager::CreateShare<CFallTetra>([sub, subIdx, thisSetIdx](CFallTetra* p) -> bool
                {
                    p->SetParamSetIndex(thisSetIdx);
                    p->SetSubParamIndex(static_cast<int>(subIdx));
                    p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight / 2, sub.ColliderDepth / 2);
                    OBJ::Transform TF = {};
                    TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                    p->SetTransform(TF);
                    return true;
                }, OBJ::TYPE::OBSTACLE);
            break;
        case OBS_TYPE::PENDULUM:
            CObjectManager::CreateShare<CPendulum>([sub, subIdx, thisSetIdx](CPendulum* p) -> bool
                {
                    p->SetParamSetIndex(thisSetIdx);
                    p->SetSubParamIndex(static_cast<int>(subIdx));
                    p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight / 2, sub.ColliderDepth / 2);
                    OBJ::Transform TF = {};
                    TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                    p->SetTransform(TF);
                    return true;
                }, OBJ::TYPE::OBSTACLE);
            break;
        case OBS_TYPE::BOOMERANG:
            CObjectManager::CreateShare<CBoomerang>([sub, subIdx, thisSetIdx](CBoomerang* p) -> bool {
                p->SetParamSetIndex(thisSetIdx);
                p->SetSubParamIndex(static_cast<int>(subIdx));
                p->SetMovePattern(sub.BoomerangMovePattern);
                p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight / 2, sub.ColliderDepth / 2);

                OBJ::Transform TF = {};
                TF.Pos = { sub.ObstacleSpawnX, 9.0f, sub.ObstacleSpawnZ };
                p->SetTransform(TF);

                // パラメータセット
                p->SetBoomerangParams(
                    sub.BoomerangOmega,
                    sub.BoomerangRadius,
                    sub.BoomerangBasePower,
                    sub.BoomerangAddBySpeed,
                    sub.BoomerangMaxFinalPower,
                    sub.BoomerangHitCooldown
                );
                return true;
                }, OBJ::TYPE::OBSTACLE);
            break;
        case OBS_TYPE::BIRDSTRIKE:
            CObjectManager::CreateShare<CBirdStrike>([sub, subIdx, thisSetIdx](CBirdStrike* p) -> bool
                {
                    p->SetParamSetIndex(thisSetIdx);
                    p->SetSubParamIndex(static_cast<int>(subIdx));
                    p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight / 2, sub.ColliderDepth / 2);
                    OBJ::Transform TF = {};
                    TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };
                    p->SetTransform(TF);
                    return true;
                }, OBJ::TYPE::OBSTACLE);
            break;
        case OBS_TYPE::BARREL:
            CObjectManager::CreateShare<CBarrel>([sub, subIdx, thisSetIdx](CBarrel* p) -> bool
                {
                    p->SetParamSetIndex(thisSetIdx);
                    p->SetSubParamIndex(static_cast<int>(subIdx));
                    // 先にコライダーを生成
                    p->FactoryCollider(sub.ColliderWidth / 2, sub.ColliderHeight / 2, sub.ColliderDepth / 2);

                    OBJ::Transform TF = {};
                    TF.Pos = { sub.ObstacleSpawnX, sub.ObstacleSpawnY, sub.ObstacleSpawnZ };

                    // 必要であれば角度設定。速度ベースにしたい場合はここを工夫
                    CBarrel::SetRotate(TF, { sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });
                    p->SetTransform(TF);

                    // 進行方向もエディタ値を使う
                    p->SetDirection({ sub.ObstacleSpeedX, sub.ObstacleSpeedY, sub.ObstacleSpeedZ });

                    const CRigidBody* const pRigidBody = useful::DownCast<CRigidBody>(p->GetCollider());
                    if (pRigidBody) {
                        pRigidBody->SetWorldTransform(TF);
                    }

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

            // manual_typeが3（BOMB）のときのみブーメラン関連パラメータを書き込む
            if (sub.ManualObstacleType == ObstacleEditer::OBS_TYPE::BOMB)
            {
                jSub["bomb_timer"] = sub.BombTimer;
            }

            // manual_typeが7（BOOMERANG）のときのみブーメラン関連パラメータを書き込む
            if (sub.ManualObstacleType == ObstacleEditer::OBS_TYPE::BOOMERANG) 
            {
                jSub["boomerang_move_pattern"] = sub.BoomerangMovePattern;
                jSub["boomerang_omega"] = sub.BoomerangOmega;
                jSub["boomerang_radius"] = sub.BoomerangRadius;
                jSub["boomerang_base_power"] = sub.BoomerangBasePower;
                jSub["boomerang_add_by_speed"] = sub.BoomerangAddBySpeed;
                jSub["boomerang_max_final_power"] = sub.BoomerangMaxFinalPower;
                jSub["boomerang_hit_cooldown"] = sub.BoomerangHitCooldown;
            }
            jParamSet["sub_params"].push_back(jSub);
        }

        jsRoot["param_sets"].push_back(jParamSet);
    }

    // 生成時間プリセットやプレイモード関連の保存
    jsRoot["spawn_time_presets"] = nlohmann::json::array();
    for (int i = 0; i < s_SpawnTimePresetCount && i < (int)s_SpawnTimePresets.size(); ++i)
    {
        jsRoot["spawn_time_presets"].push_back(s_SpawnTimePresets[i]);
    }
    jsRoot["spawn_player_thresholds"] = nlohmann::json::array();
    for (int i = 0; i < s_SpawnTimePresetCount && i < (int)s_SpawnPlayerThresholds.size(); ++i)
    {
        jsRoot["spawn_player_thresholds"].push_back(s_SpawnPlayerThresholds[i]);
    }

    jsRoot["spawn_enable_time"] = 3.0f;
    jsRoot["preset_count"] = s_SpawnTimePresetCount;

    jsRoot["falltetra_decay_value"] = s_DecayValue;

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
    m_PlayModeElapsedTime = 0.0f;

#ifdef _DEBUG
    m_PlayMode = false;
#endif

#ifdef _RELEASE
    m_PlayMode = true;
#endif

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
                        sub.ObstacleSpawnZ = jSub.value("spawnZ", 0.0f);
                        sub.ObstacleSpeedX = jSub.value("speedX", 0.0f);
                        sub.ObstacleSpeedY = jSub.value("speedY", 0.0f);
                        sub.ObstacleSpeedZ = jSub.value("speedZ", -5.0f);
                        sub.ColliderWidth = jSub.value("collider_width", 3.0f);
                        sub.ColliderHeight = jSub.value("collider_height", 3.0f);
                        sub.ColliderDepth = jSub.value("collider_depth", 3.0f);
                        int manualTypeValue = jSub.value("manual_type", static_cast<int>(OBS_TYPE::NONE)); //OBS_TYPEに変換する
                        sub.ManualObstacleType = static_cast<OBS_TYPE>(manualTypeValue);
                        sub.BoomerangMovePattern = jSub.value("boomerang_move_pattern", 0);
                        sub.BombTimer = jSub.value("bomb_timer", 300);
                        sub.BoomerangOmega = jSub.value("boomerang_omega", 1.0f);
                        sub.BoomerangRadius = jSub.value("boomerang_radius", 12.0f);
                        sub.BoomerangBasePower = jSub.value("boomerang_base_power", 20.0f);
                        sub.BoomerangAddBySpeed = jSub.value("boomerang_add_by_speed", 80.0f);
                        sub.BoomerangMaxFinalPower = jSub.value("boomerang_max_final_power", 350.0f);
                        sub.BoomerangHitCooldown = jSub.value("boomerang_hit_cooldown", 10);
                        m_ParamSets[i].subParams.push_back(sub);
                    }
                }
            }
        }
    }

    if (jsRoot.contains("spawn_player_thresholds") && jsRoot["spawn_player_thresholds"].is_array())
    {
        int arrSize = jsRoot["spawn_player_thresholds"].size();
        for (int i = 0; i < arrSize && i < SPAWN_PRESET_MAX; ++i)
        {
            s_SpawnPlayerThresholds[i] = jsRoot["spawn_player_thresholds"][i].get<int>();
        }
    }
    else
    {
        s_SpawnPlayerThresholds.assign(SPAWN_PRESET_MAX, 4);
    }

    if (jsRoot.contains("falltetra_decay_value")) 
    {
        s_DecayValue = jsRoot["falltetra_decay_value"].get<float>();
    }
    else {
        s_DecayValue = 0.3f; // デフォルト値
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

    if ((int)s_ForcedParamSetIndices.size() != s_SpawnTimePresetCount)
    {
        s_ForcedParamSetIndices.resize(s_SpawnTimePresetCount, 0);
    }

    // すべてのParamSetIdx, subParamIdxペアをリスト化する
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

    // 乱数生成の準備
    std::random_device randomdevice;
    std::mt19937 randomnengine(randomdevice());

    // 選択履歴
    std::pair<int, int> lastPair = { -1, -1 };
    int lastParamSetIdx = -1;

    for (int presetIndex = 0; presetIndex < s_SpawnTimePresetCount; ++presetIndex)
    {
        std::pair<int, int> selectedPair;
        int paramSetForce = s_ForcedParamSetIndices[presetIndex]; // 0はランダム抽選, 1～5:指定のparam set

        if (paramSetForce >= 1 && paramSetForce <= PARAM_SET_MAX) 
        {
            // Param Setが指定されている場合
            int pIdx = paramSetForce - 1;
            if (pIdx < (int)m_ParamSets.size() && !m_ParamSets[pIdx].subParams.empty())
            {
                // subParamをランダムで選ぶ
                std::uniform_int_distribution<int> dist(0, (int)m_ParamSets[pIdx].subParams.size() - 1);
                int subIdx = dist(randomnengine);
                selectedPair = { pIdx, subIdx };
            }
            else
            {
                // パラメータセットが無効な場合
                selectedPair = { -1, -1 };
            }
        } 
        else 
        {
            // ランダム抽選
            bool isValidSelection = false;
            while (!isValidSelection)
            {
                std::uniform_int_distribution<int> dist(0, (int)allPairs.size() - 1);
                selectedPair = allPairs[dist(randomnengine)];
                isValidSelection = (selectedPair != lastPair) && (selectedPair.first != lastParamSetIdx);
            }
        }
        // 選択されたペアを保存
        s_AssignedSpawnParamIndices[presetIndex] = selectedPair;

        // 該当するプリセット時間を適用
        int presetTimeIndex = presetIndex % s_SpawnTimePresets.size();
        s_AssignedSpawnTimes[presetIndex] = s_SpawnTimePresets[presetTimeIndex];

        // スポーンフラグを初期化させる
        s_SpawnedFlags[presetIndex] = false;

        // 現在のペアとセットインデックスを次回のために記憶する
        lastPair = selectedPair;
        lastParamSetIdx = selectedPair.first;
    }
}

void ObstacleEditer::ShowGlobalGimmickSettingsWindow()
{
    static bool show = true; // 必要に応じて他所で切り替えてください（常時表示ならstatic不要）

    // 必要ならウィンドウタイトルで開閉（外部から show フラグ制御OK）
    if (ImGui::Begin(reinterpret_cast<const char*>(u8"ギミック効果全体設定"), &show))
    {
        ImGui::Text(reinterpret_cast<const char*>(u8"ドッスン直撃時のプレイヤー移動減速値"));
        ImGui::DragFloat(reinterpret_cast<const char*>(u8"減速値"), &s_DecayValue, 0.01f, 0.0f, 1.0f);

        auto& playerList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);
        for (auto& playerObj : playerList)
        {
            auto player = std::dynamic_pointer_cast<CPlayer>(playerObj);
            if (!player) continue;
            auto fallTetra = player->GetFallTetraBehavior();
            if (fallTetra) {
                fallTetra->SetDecayValue(s_DecayValue);
            }
        }
    }
    ImGui::End();
}