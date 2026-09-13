#include "AoECircle.h"

void AoECircle::Draw() const
{
    // チャージ進行度 (0.0 ～ 1.0)
    float progress = timer_ / duration_;
    if (progress > 1.0f) progress = 1.0f;

    constexpr int DIV = 32; // 円の分割数
    // チラつき防止で床よりわずかに浮かせる
    float groundY = pos_.y + 0.1f;

    // アルファブレンディング有効化
    SetWriteZBuffer3D(FALSE);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);

    // ベース範囲円
    {
        VERTEX3D vertices[DIV + 1];
        WORD indices[DIV * 3];

        vertices[0].pos = VGet(pos_.x, groundY, pos_.z);
        vertices[0].dif = GetColorU8(255, 50, 50, 150);
        vertices[0].norm = VGet(0.0f, 1.0f, 0.0f);

        for (int i = 0; i < DIV; ++i)
        {
            float angle = DX_PI_F * 2.0f * i / DIV;
            vertices[i + 1].pos = VGet(pos_.x + cosf(angle) * radius_, groundY, pos_.z + sinf(angle) * radius_);
            vertices[i + 1].dif = GetColorU8(255, 50, 50, 150);
            vertices[i + 1].norm = VGet(0.0f, 1.0f, 0.0f);
        }
        for (int i = 0; i < DIV; ++i)
        {
            indices[i * 3 + 0] = 0;
            indices[i * 3 + 1] = i + 1;
            indices[i * 3 + 2] = (i == DIV - 1) ? 1 : (i + 2);
        }

        DrawPolygonIndexed3D(vertices, DIV + 1, indices, DIV, DX_NONE_GRAPH, TRUE);
    }

    // タイマー円
    float currentRadius = radius_ * progress; // 時間経過で 0 -> 最大半径 へ拡張
    if (currentRadius > 0.001f)
    {
        VERTEX3D vertices[DIV + 1];
        WORD indices[DIV * 3];

        // 重なりチラつきを防ぐため、ベースより少し上(0.05f)に配置
        float timerY = groundY + 0.05f;

        vertices[0].pos = VGet(pos_.x, timerY, pos_.z);
        vertices[0].dif = GetColorU8(255, 60, 60, 225); // 濃い赤
        vertices[0].norm = VGet(0.0f, 1.0f, 0.0f);

        for (int i = 0; i < DIV; ++i)
        {
            float angle = DX_PI_F * 2.0f * i / DIV;
            vertices[i + 1].pos = VGet(pos_.x + cosf(angle) * currentRadius, timerY, pos_.z + sinf(angle) * currentRadius);
            vertices[i + 1].dif = GetColorU8(255, 60, 60, 225);
            vertices[i + 1].norm = VGet(0.0f, 1.0f, 0.0f);
        }

        for (int i = 0; i < DIV; ++i)
        {
            indices[i * 3 + 0] = 0;
            indices[i * 3 + 1] = i + 1;
            indices[i * 3 + 2] = (i == DIV - 1) ? 1 : (i + 2);
        }

        DrawPolygonIndexed3D(vertices, DIV + 1, indices, DIV, DX_NONE_GRAPH, TRUE);
    }

    // 外周線の描画
    //unsigned int outlineColor = GetColor(255, 120, 120);
    //for (int i = 0; i < DIV; ++i)
    //{
    //    float angle1 = DX_PI_F * 2.0f * i / DIV;
    //    float angle2 = DX_PI_F * 2.0f * (i + 1) / DIV;

    //    VECTOR p1 = VGet(pos_.x + cosf(angle1) * radius_, groundY + 0.1f, pos_.z + sinf(angle1) * radius_);
    //    VECTOR p2 = VGet(pos_.x + cosf(angle2) * radius_, groundY + 0.1f, pos_.z + sinf(angle2) * radius_);

    //    DrawLine3D(p1, p2, outlineColor);
    //}

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    SetWriteZBuffer3D(TRUE);
}