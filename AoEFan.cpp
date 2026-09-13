#include "AoEFan.h"

void AoEFan::Draw() const
{
    float progress = timer_ / duration_;
    if (progress > 1.0f) progress = 1.0f;

    constexpr int DIV = 16;
    float radAngle = angle_ * DX_PI_F / 180.0f;
    float startAngle = atan2f(dir_.x, dir_.z) - (radAngle * 0.5f);
    float groundY = pos_.y + 0.1f;

    SetWriteZBuffer3D(FALSE);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);

    // 1. 下地（扇状ポリゴン）
    VERTEX3D v[DIV + 2];
    WORD indices[DIV * 3];

    v[0].pos = VGet(pos_.x, groundY, pos_.z);
    v[0].dif = GetColorU8(255, 50, 50, 150);
    v[0].norm = VGet(0.0f, 1.0f, 0.0f);

    for (int i = 0; i <= DIV; ++i)
    {
        float a = startAngle + (radAngle * i / DIV);
        v[i + 1].pos = VGet(pos_.x + sinf(a) * radius_, groundY, pos_.z + cosf(a) * radius_);
        v[i + 1].dif = GetColorU8(255, 50, 50, 150);
        v[i + 1].norm = VGet(0.0f, 1.0f, 0.0f);
    }
    for (int i = 0; i < DIV; ++i)
    {
        indices[i * 3 + 0] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = i + 2;
    }
    DrawPolygonIndexed3D(v, DIV + 2, indices, DIV, DX_NONE_GRAPH, TRUE);

    // 2. 進行ゲージ（中心から外側へ広がる扇）
    float currentRadius = radius_ * progress;
    if (currentRadius > 0.001f)
    {
        VERTEX3D vp[DIV + 2];
        vp[0].pos = VGet(pos_.x, groundY + 0.05f, pos_.z);
        vp[0].dif = GetColorU8(255, 60, 60, 225);
        vp[0].norm = VGet(0.0f, 1.0f, 0.0f);

        for (int i = 0; i <= DIV; ++i)
        {
            float a = startAngle + (radAngle * i / DIV);
            vp[i + 1].pos = VGet(pos_.x + sinf(a) * currentRadius, groundY + 0.05f, pos_.z + cosf(a) * currentRadius);
            vp[i + 1].dif = GetColorU8(255, 60, 60, 225);
            vp[i + 1].norm = VGet(0.0f, 1.0f, 0.0f);
        }
        DrawPolygonIndexed3D(vp, DIV + 2, indices, DIV, DX_NONE_GRAPH, TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    SetWriteZBuffer3D(TRUE);
}
