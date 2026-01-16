//============================================================================
//
// ブーメラン [boomerang.cpp]
// Author : 大竹熙
//
//============================================================================

//****************************************************
// インクルードファイル
//****************************************************
#include "boomerang.h"

// 物理挙動作成のため
#include "API.world.h"
#include "API.collision.h"

// エフェクト
#include "dust.h"

//****************************************************
// usingディレクティブ
//****************************************************
using namespace DirectX;
using namespace useful;

//****************************************************
// 無名名前空間の定義
//****************************************************
namespace
{
    // フィールドサイズ
    float g_fFieldSpan = 15.0f;
    float g_fFieldHalf = g_fFieldSpan * 0.5f;

    // 高さ
    float g_fBoomerangY = 9.0f; 

    // ブーメランパラメータ
    namespace BoomerangParams
    {
        // ================================
        // --- ブーメランの軌道パラメータ ---
        // ================================

        // 速度
        const float Omega = 2.0;

        // 半径（弧の大きさ）
        const float Radius = g_fFieldSpan * 0.8f;

        // 1回の弧で動かす角度（ラジアン）
        const float ArcAngle = DirectX::XM_PI;


        // =======================
        // --- 当たり判定関連 ---
        // =======================

        // 擬似速度計算用のデルタタイム
        const float Dt = 1.0f / 60.0f;

        // 擬似速度の上限（安全装置）
        const float MaxSpeed = 20.0f;

        // ヒット後のクールタイム
        const int HitCooldown = 10;


        // ================================
        // --- 吹っ飛びパワー調整 ---
        // ================================

        // 基本吹っ飛びパワー
        const float BasePower = 20.0f;

        // 速度依存の加算パワー
        const float AddBySpeed = 80.0f;

        // 最終的な吹っ飛びパワーの上限
        const float MaxFinalPower = 350.0f;
    }

    // 位置表示
    void Print_Pos(const OBJ::Transform& TF)
    {
        useful::MIS::MyImGuiShortcut_BeginWindow("Any Debug");
        if (ImGui::TreeNodeEx("Boomerang", ImGuiTreeNodeFlags_OpenOnArrow))
        {
            ImGui::Text("Boomerang Pos X: %.2f", TF.Pos.x);
            ImGui::Text("Boomerang Pos Y: %.2f", TF.Pos.y);
            ImGui::Text("Boomerang Pos Z: %.2f", TF.Pos.z);
            ImGui::TreePop();
        }
        ImGui::End();
    }
}

//============================================================================
// デフォルトコンストラクタ
//============================================================================
CBoomerang::CBoomerang(OBJ::TYPE Type, OBJ::LAYER Layer)
    : CObstacle(Type, Layer, Obstacle::OBSTACLE_TYPE::PERIMETER)
    , m_Direction(VEC3_ZERO_INIT)
    , m_Time(0.0f)
    , m_StartAngle(0.0f)
    , m_EndAngle(0.0f)
    , m_Center({ 0.0f, 0.0f, 0.0f })
{}

//============================================================================
// デストラクタ
//============================================================================
CBoomerang::~CBoomerang()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CBoomerang::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
    SetCollider(CRigidBody::CreateRigidBody(
        GetTransform(),
        Collision::SHAPETYPE::SPHERE,
        fWidth, fHeight, fDepth));

    m_pRB = useful::DownCast<CRigidBody>(GetCollider());

    btRigidBody* rb = m_pRB->GetRigidBody();

    rb->setCollisionFlags(rb->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    rb->setActivationState(DISABLE_DEACTIVATION);
    rb->setGravity(btVector3(0, 0, 0));

    Appear();
}

//============================================================================
// 更新処理
//============================================================================
void CBoomerang::Update()
{
    const float dt = 1.0f / 60.0f;
    m_Time += dt;

    // 挙動
    Action();

    // 戻る
    Loop();

    CheckHitPlayer();

    // 物理オブジェクト用の更新
    CPhysicsObject::Update();
}

//============================================================================
// 描画処理
//============================================================================
void CBoomerang::Draw()
{
    CPhysicsObject::Draw();
}

//============================================================================
// 出現
//============================================================================
void CBoomerang::Appear()
{
    OBJ::Transform TF{};

    const float R = BoomerangParams::Radius;
    const float H = g_fBoomerangY;

    // 0: 奥→手前, 1: 手前→奥, 2: 右→左, 3: 左→右
    const int nPattern = rand() % 4;

    switch (nPattern)
    {
    case 0: // 奥→手前
        m_StartAngle = DirectX::XM_PI;
        m_EndAngle = DirectX::XM_PI * 2.0f;
        m_Center = { 0.0f, H, g_fFieldHalf + R };
        break;

    case 1: // 手前→奥
        m_StartAngle = 0.0f;
        m_EndAngle = DirectX::XM_PI;
        m_Center = { 0.0f, H, -g_fFieldHalf - R };
        break;

    case 2: // 右→左
        m_StartAngle = DirectX::XM_PIDIV2;
        m_EndAngle = DirectX::XM_PI * 1.5f;
        m_Center = { g_fFieldHalf + R, H, 0.0f };
        break;

    case 3: // 左→右
        m_StartAngle = DirectX::XM_PIDIV2 + DirectX::XM_PI;
        m_EndAngle = DirectX::XM_PI * 1.5f + DirectX::XM_PI;
        m_Center = { -g_fFieldHalf - R, H, 0.0f };
        break;
    }

    // 開始位置
    const float x = m_Center.x + R * cosf(m_StartAngle);
    const float z = m_Center.z + R * sinf(m_StartAngle);

    TF.Pos = { x, H, z };
    m_Time = 0.0f;

    if (m_pRB)
        m_pRB->SetWorldTransform(TF);
}

//============================================================================
// 挙動
//============================================================================
void CBoomerang::Action()
{
    if (!m_pRB) return;

    const float R = BoomerangParams::Radius;

    // 経過時間に応じて角度を進める
    const float travel = BoomerangParams::Omega * m_Time;

    // StartAngle → EndAngle へ向かって角度を更新
    float theta = m_StartAngle;

    if (m_EndAngle >= m_StartAngle)
    {
        theta = m_StartAngle + travel;
        if (theta > m_EndAngle) theta = m_EndAngle;
    }
    else
    {
        theta = m_StartAngle - travel;
        if (theta < m_EndAngle) theta = m_EndAngle;
    }

    OBJ::Transform TF{};

    // XZ 平面の円軌道上の位置を計算
    TF.Pos.x = m_Center.x + R * cosf(theta);
    TF.Pos.z = m_Center.z + R * sinf(theta);
    TF.Pos.y = m_Center.y;

    m_pRB->SetWorldTransform(TF);
}

//============================================================================
// 戻る
//============================================================================
void CBoomerang::Loop()
{
    if (!m_pRB) return;

    // 1回の弧で必要な時間 = ArcAngle / Omega
    const float fullTime = BoomerangParams::ArcAngle / BoomerangParams::Omega;

    OBJ::Transform TF{};
    m_pRB->GetWorldTransform(TF);

    if (m_Time >= fullTime)
    {
        m_Time = 0.0f;

        // 出し直し
        Appear();

        CDust::GenerateSpread(TF.Pos, 10);
    }

    Print_Pos(TF);
}

//============================================================================
// プレイヤーとの当たり判定
//============================================================================
void CBoomerang::CheckHitPlayer()
{
    if (!m_pRB) return;

    CRigidBody* const pBoomRB = m_pRB;

    OBJ::Transform tf{};
    pBoomRB->GetWorldTransform(tf);

    // 擬似速度
    btVector3 pseudoVel(0, 0, 0);
    if (m_hasPrevPos)
    {
        pseudoVel = btVector3(
            tf.Pos.x - m_prevPos.x,
            tf.Pos.y - m_prevPos.y,
            tf.Pos.z - m_prevPos.z
        ) / BoomerangParams::Dt;
    }

    float speed = pseudoVel.length();
    if (speed > BoomerangParams::MaxSpeed)
        speed = BoomerangParams::MaxSpeed;

    // 折り返し検出
    float velX = pseudoVel.x();
    if (m_prevVelX * velX < 0.0f)
    {
        m_HitPlayers.clear();
    }
    m_prevVelX = velX;

    // クールタイム
    if (m_HitCooldown > 0)
    {
        --m_HitCooldown;
        m_prevPos = tf.Pos;
        m_hasPrevPos = true;
        return;
    }

    // プレイヤー全員チェック
    const auto& rPlayerList = CObjectManager::RefInstance().RefListShare(OBJ::TYPE::PLAYER);
    for (const auto& rIt : rPlayerList)
    {
        CPlayer* pPlayer = dynamic_cast<CPlayer*>(rIt.get());
        if (!pPlayer) continue;

        CRigidBody* const pPlayerRB = useful::DownCast<CRigidBody>(pPlayer->GetCollider());
        if (!pPlayerRB) continue;

        if (m_HitPlayers.count(pPlayer) > 0)
            continue;

        // 接触判定
        Collision::MyContactCallbackRigidBodyAndRigidBody callback(pBoomRB, pPlayerRB);
        CWorld::RefInstance().RefDynamicsWorldConst()->contactPairTest(
            pBoomRB->GetRigidBody(),
            pPlayerRB->GetRigidBody(),
            callback
        );
        if (!callback.m_bHit) continue;

        // ヒット記録
        m_HitPlayers.insert(pPlayer);

        // 吹っ飛び方向
        btVector3 dir = pseudoVel;
        if (dir.length() < 1e-6f)
            dir = btVector3(1, 0, 0);
        dir = dir.normalized();

        // パワー計算
        float t = speed / BoomerangParams::MaxSpeed;

        float power = BoomerangParams::BasePower
            + BoomerangParams::AddBySpeed * t;

        power = std::clamp(power, 0.0f, BoomerangParams::MaxFinalPower);

        // 少し浮かせる
        {
            OBJ::Transform tfp = {};
            pPlayerRB->GetWorldTransform(tfp);
            tfp.Pos.y += 0.1f;
            pPlayerRB->SetWorldTransform(tfp);
        }

        // インパルス
        pPlayerRB->SetActive();
        pPlayerRB->SetImpulse(dir * power);

        // 速度制限
        {
            btVector3 v = pPlayerRB->GetLinearVelocity();
            if (v.getY() < 0.0f) v.setY(0.0f);

            const float MaxPlayerSpeed = 70.0f;
            float len = v.length();
            if (len > MaxPlayerSpeed)
                v *= (MaxPlayerSpeed / len);

            pPlayerRB->SetLinearVelocity(v);
        }

        m_HitCooldown = 25;
        break;
    }

    m_prevPos = tf.Pos;
    m_hasPrevPos = true;
}