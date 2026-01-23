//============================================================================
// 
// ブーメラン [boomerang.h]
// Author : 大竹熙
// 
//============================================================================

#pragma once

//****************************************************
// インクルードファイル
//****************************************************
#include "obstacle.h"
#include "player.h"
#include "API.rigidbody.h"
#include <unordered_set>

//****************************************************
// バークラスの定義
//****************************************************
class CBoomerang : public CObstacle
{
public:

    //****************************************************
    // special function
    //****************************************************
    CBoomerang(OBJ::TYPE Type, OBJ::LAYER Layer); // デフォルトコンストラクタ
    ~CBoomerang() override;                       // デストラクタ

    //****************************************************
    // function
    //****************************************************

    // コライダーのファクトリ
    void FactoryCollider(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f) override;

    // 更新処理
    void Update() override;

    // 描画処理
    void Draw() override;

    // パラメータの編集
    void EditParam() override { int i = 0; }

    // 進行方向の設定
    inline void SetDirection(const DirectX::XMFLOAT3& Direction) { m_Direction = Direction; }
    void SetBoomerangParams(float omega, float radius, float basePower, float addBySpeed, float maxFinalPower, int hitCooldown);
    void SetMovePattern(int pattern) { m_MovePattern = pattern; }
    int m_MovePattern = 0;

    /* ！！！ 回転の半径の取得 ！！！ */
	inline float GetRadius() const { return m_Radius; }

    /* ！！！ 回転の中心点の取得 ！！！ */
	inline const DirectX::XMFLOAT3& GetCenter() const { return m_Center; }

    /* ！！！ 弧の開始角度の取得 ！！！ */
    inline float GetStartAngle() const { return m_StartAngle; }

private:

    //****************************************************
    // function
    //****************************************************
    void Appear();          // 出現
    void Action();          // 挙動（ブーメラン軌道）
    void Loop();            // 戻る
    void CheckHitPlayer();  // プレイヤーとの当たり判定

    //****************************************************
    // data
    //****************************************************
    DirectX::XMFLOAT3 m_Direction;   // 進行方向（未使用でも構造合わせ）
    float m_Time;                    // 経過時間
    int m_HitCooldown = 0;           // ヒットクールタイム

    DirectX::XMFLOAT3 m_prevPos = { 0.0f, 0.0f, 0.0f }; // 前フレーム位置
    bool m_hasPrevPos = false;                          // 初回記録済み

    float m_prevVelX = 0.0f;          // 折り返し検出用
    CRigidBody* m_pRB = nullptr;      // 物理ボディ

    std::unordered_set<CPlayer*> m_HitPlayers; // ヒット済みプレイヤー

    float m_StartAngle;  // 弧の開始角度（ラジアン）
    float m_EndAngle;    // 弧の終了角度（ラジアン）
    float m_Omega = 1.0f;
    float m_Radius = 12.0f;
    float m_BasePower = 20.0f;
    float m_AddBySpeed = 80.0f;
    float m_MaxFinalPower = 350.0f;
    int   m_CustomHitCooldown = 10;
    DirectX::XMFLOAT3 m_Center; // 円の中心（XZ平面の支点）
};