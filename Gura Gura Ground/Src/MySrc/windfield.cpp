//============================================================================
// 
// 風ステージの処理 [windfield.cpp]
// Author : 千葉
// 
//============================================================================

//****************************************************
// 自身のインクルード
//****************************************************
#include "windfield.h"

//****************************************************
// 自前方宣言のインクルード
//****************************************************
#include "player.h"
#include "enemy1.h"

//****************************************************
// 必要なインクルード
//****************************************************
#include "API.gltf.manager.h"

// コライダーの作成用
#include "API.rigidbody.h"
#include <numbers> 
#include "API.input.manager.h"

#include "API.texture.manager.h"
#include "API.window.h"
#include "API.singleton.h"

//================================================
//名前空間（無名）
namespace
{
    btVector3 INIT = { 0.0f, 0.0f, 0.0f };
    btVector3 INIT_PLYER_VELOCITY = { 0.0f, 1.144f, 0.0f };
    const float ANGLE = (float)std::numbers::pi * 0.5f;
    const float AIR_SPEED = 0.55f;

    int PLAYER_SIZE;
    int CPU_SIZE;
}

//================================================
using namespace useful;


//============================================================================
// デフォルトコンストラクタ
//============================================================================
CWindField::CWindField(OBJ::TYPE Type, OBJ::LAYER Layer)
    : CPhysicsModel(Type, Layer)
    , m_WindowRotationAngle(ANGLE)
    , m_SaverCurrentVel(INIT_PLYER_VELOCITY)
{
    SetModel(CGltfManager::RefInstance().RefRegistry().BindAtKey("Field"));
    SetModelOffset({ 1.15f, 0.8f, -0.3f });

    auto pArrow = CObjectManager::CreateRaw<CHud>(OBJ::TYPE::NONE, OBJ::LAYER::UI);

    // 最初は右向き矢印をセット
    pArrow->SetTexture(CTextureManager::RefInstance().RefRegistry().BindAtKey("WindArrow_Right"));

    OBJ::Transform arrowTF =
    {
        { 1100.0f, 150.0f, 0.0f },
        { 0.0f,   0.0f,   0.0f, 0.0f },
        { 100.0f, 100.0f, 0.0f }
    };

    pArrow->SetTransform(arrowTF);
    pArrow->SetTransformTarget(arrowTF);

    DirectX::XMFLOAT4 col = pArrow->GetCol();
    col.w = 0.0f;
    pArrow->SetCol(col);
    pArrow->SetColTarget(col);

    m_pWindArrow = pArrow;
}

//============================================================================
// デストラクタ
//============================================================================
CWindField::~CWindField()
{}

//============================================================================
// コライダーのファクトリ
//============================================================================
void CWindField::FactoryCollider(float fWidth, float fHeight, float fDepth)
{
    SetCollider(CRigidBody::CreateRigidBody(GetTransform(), Collision::SHAPETYPE::BOX, fWidth, fHeight, fDepth));

    CRigidBody* pRB = useful::DownCast<CRigidBody>(GetCollider());
    pRB->SetMass(0.0f);
    pRB->SetRestitution(1.0f);
}

//============================================================================
// 更新処理
//============================================================================
void CWindField::Update()
{
    UpdatePlayersSystem();

    // 物理モデル用の更新
    CPhysicsModel::Update();

    // ★ 矢印 HUD の更新
    if (m_pWindArrow)
        m_pWindArrow->Update();
}

//============================================================================
// 全てのプレイヤーシステムの更新処理
//============================================================================
void CWindField::UpdatePlayersSystem()
{
    if (m_pwPlayer.empty())
    {
        SearchPYInfo();
        PLAYER_SIZE = (int)m_pwPlayer.size();
    }
    else if (m_pwEnemyPlayer.empty())
    {
        SearchCPUInfo();
        CPU_SIZE = (int)m_pwEnemyPlayer.size();
    }
    else
    {
        Window();
    }
}

//============================================================================
// 各情報を探す処理
//============================================================================
void CWindField::SearchPYInfo()
{
    const auto playerlist = CObjectManager::RefInstance().RefInstance().RefListShare(OBJ::TYPE::PLAYER);

    for (auto Obj : playerlist)
    {
        auto pPlayer = std::dynamic_pointer_cast<CPlayer>(Obj);
        m_pwPlayer.push_back(pPlayer);
    }
}

//============================================================================
// 各情報を探す処理
//============================================================================
void CWindField::SearchCPUInfo()
{
    const auto enemyplayerlist = CObjectManager::RefInstance().RefInstance().RefListShare(OBJ::TYPE::CPU);

    for (auto Obj1 : enemyplayerlist)
    {
        auto pEnemyPlayer = std::dynamic_pointer_cast<CEnemyPlayer>(Obj1);
        m_pwEnemyPlayer.push_back(pEnemyPlayer);
    }
}

//============================================================================
// 移動させる処理
//============================================================================
void CWindField::MovePlayer(float Angle, float speed, int PlayerSize, int CPUSize)
{
    for (int nPlayerCount = 0; nPlayerCount < PlayerSize; ++nPlayerCount)
    {
        auto sp = m_pwPlayer[nPlayerCount].lock();
        if (!sp) continue;

        CRigidBody* pRB = DownCast<CRigidBody>(sp->GetCollider());
        if (!pRB) continue;

        ApplyWindToBody_PY(pRB, Angle, speed, m_pwPlayer[nPlayerCount]);
    }

    for (int CPUCount = 0; CPUCount < CPUSize; ++CPUCount)
    {
        auto CUP = m_pwEnemyPlayer[CPUCount].lock();
        if (!CUP) continue;

        CRigidBody* pRB = DownCast<CRigidBody>(CUP->GetCollider());
        if (!pRB) continue;

        ApplyWindToBody_CPU(pRB, Angle, speed, m_pwEnemyPlayer[CPUCount]);
    }
}

//============================================================================
// プレイヤー用
//============================================================================
void CWindField::ApplyWindToBody_PY(CRigidBody* pRB, float Angle, float speed, std::weak_ptr<CPlayer> pwPlayer)
{
    const auto& opDirection =
        CInputManager::RefInstance().ConvertInputToMoveDirection(pwPlayer.lock()->GetIdxPlayer());

    if (!CheckLand(pwPlayer))
    {
        speed = speed * AIR_SPEED;
    }

    if (opDirection)
    {
        speed = speed * 1.15f;
    }

    ApplyWindCommon(pRB, Angle, speed);
}

//============================================================================
// CPU用
//============================================================================
void CWindField::ApplyWindToBody_CPU(CRigidBody* pRB, float Angle, float speed, std::weak_ptr<CEnemyPlayer> pwCPU)
{
    if (!CheckLand(pwCPU))
    {
        speed = speed * AIR_SPEED;
    }

    ApplyWindCommon(pRB, Angle, speed);
}

//============================================================================
//風の影響を適用する
//============================================================================
void CWindField::ApplyWindCommon(CRigidBody* pRB, float Angle, float speed)
{
    btVector3 rCurrentVel = pRB->GetLinearVelocity();

    if (rCurrentVel == INIT)
    {
        rCurrentVel = m_SaverCurrentVel;
    }

    pRB->SetActive();

    btVector3 newVel = rCurrentVel;

    btVector3 windDir(sinf(Angle), 0.0f, cosf(Angle));

    newVel += (windDir * speed);

    float dot = rCurrentVel.dot(windDir);
    if (dot < 0.0f) {
        float resistance = 0.5f;
        newVel = newVel.lerp(windDir * speed, resistance);
    }

    newVel.setY(rCurrentVel.getY());

    pRB->SetLinearVelocity(newVel);
}

//============================================================================
//風のギミックの処理
//============================================================================
void CWindField::Window()
{
    ++m_Parameter.m_Timer;

    if (!m_pWindArrow)
        return;

    const float arrowW = 400.0f;
    const float arrowH = 400.0f;

    auto getDirIndex = [](float angle)
    {
        int dir = (int)roundf(angle / ANGLE) % 4;
        if (dir < 0) dir += 4;
        return dir;
    };

    auto setArrowPos = [&](int dir)
    {
        OBJ::Transform tf = m_pWindArrow->GetTransform();
        tf.Size = { arrowW, arrowH, 0.0f };

        switch (dir)
        {
        case 0: tf.Pos = { 950.0f, 1000.0f, 0.0f }; break; // Up    → 下辺中央
        case 1: tf.Pos = { 200.0f, 500.0f, 0.0f }; break;  // Right → 左辺中央
        case 2: tf.Pos = { 950.0f, 50.0f,  0.0f }; break;  // Down  → 上辺中央
        case 3: tf.Pos = { 1700.0f, 500.0f, 0.0f }; break; // Left  → 右辺中央
        }

        m_pWindArrow->SetTransform(tf);
        m_pWindArrow->SetTransformTarget(tf);
    };

    auto setArrowTexture = [&](int dir)
    {
        static const char* keys[4] =
        {
            "WindArrow_Up",
            "WindArrow_Right",
            "WindArrow_Down",
            "WindArrow_Left"
        };

        m_pWindArrow->SetTexture(
            CTextureManager::RefInstance().RefRegistry().BindAtKey(keys[dir])
        );
    };

    auto setArrowAlpha = [&](float alpha)
    {
        DirectX::XMFLOAT4 col = m_pWindArrow->GetCol();
        col.w = alpha;
        m_pWindArrow->SetCol(col);
        m_pWindArrow->SetColTarget(col);
    };

    // ------------------------------
    // 風が吹いていないフェーズ（予告）
    // ------------------------------
    if (!m_Parameter.m_IsBlowing)
    {
        if (m_Parameter.m_Timer >= (m_Parameter.m_StopTime - 120))
        {
            int dir = getDirIndex(m_WindowRotationAngle);

            setArrowTexture(dir);
            setArrowPos(dir);

            // 点滅
            int t = (int)m_Parameter.m_Timer;
            float alpha = ((t / 10) % 2 == 0) ? 1.0f : 0.0f;
            setArrowAlpha(alpha);
        }

        if (m_Parameter.m_Timer >= m_Parameter.m_StopTime)
        {
            m_Parameter.m_Timer = 0.0f;
            m_Parameter.m_IsBlowing = true;

            m_Parameter.m_WindAngle = m_WindowRotationAngle;
            m_Parameter.m_WindSpeed = 0.9f;

            m_WindowRotationAngle += ANGLE;
            if (m_WindowRotationAngle >= ANGLE * 4.0f)
                m_WindowRotationAngle = 0.0f;
        }

        return;
    }

    // ------------------------------
    // 風が吹いているフェーズ
    // ------------------------------
    MovePlayer(m_Parameter.m_WindAngle, m_Parameter.m_WindSpeed, PLAYER_SIZE, CPU_SIZE);

    {
        int dir = getDirIndex(m_Parameter.m_WindAngle);

        setArrowTexture(dir);
        setArrowPos(dir);
        setArrowAlpha(1.0f);
    }

    if (m_Parameter.m_Timer >= m_Parameter.m_BlowTime)
    {
        m_Parameter.m_Timer = 0.0f;
        m_Parameter.m_IsBlowing = false;

        setArrowAlpha(0.0f);
    }
}

//============================================================================
// 描画処理
//============================================================================
void CWindField::Draw()
{
    CPhysicsModel::Draw();

    // ★ 矢印 HUD の描画
    if (m_pWindArrow)
        m_pWindArrow->Draw();
}