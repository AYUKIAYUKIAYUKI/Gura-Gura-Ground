//============================================================================
// 
// 氷フィールド [field_ice.h]
// Author : 大竹熙
// 
//============================================================================

#pragma once
#include "field.h"

class CFieldIce : public CField
{
public:
    CFieldIce(OBJ::TYPE Type, OBJ::LAYER Layer);
    ~CFieldIce() override;

    // コライダー生成
    void FactoryCollider(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f) override;

    // 更新処理
    void Update() override;

    // 描画処理
    void Draw() override;
};