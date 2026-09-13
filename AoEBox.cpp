#include "AoEBox.h"

void AoEBox::Draw() const
{
    float progress = timer_ / duration_;
    if (progress > 1.0f) progress = 1.0f;

    // 照射方向から右方向ベクトルを算出
    VECTOR right = VGet(dir_.z, 0.0f, -dir_.x);
    float halfWidth = width_ * 0.5f;
    float groundY = pos_.y + 0.1f;

    // 4頂点の算出 (ローカル -> ワールド)
    VECTOR p[4];
    p[0] = VAdd(pos_, VScale(right, -halfWidth)); // 左手前
    p[1] = VAdd(pos_, VScale(right, halfWidth));  // 右手前
    p[2] = VAdd(p[1], VScale(dir_, radius_));  // 右奥
    p[3] = VAdd(p[0], VScale(dir_, radius_));  // 左奥

    SetWriteZBuffer3D(FALSE);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);

    // 1. 下地（薄い赤）
    VERTEX3D v[4];
    for (int i = 0; i < 4; ++i)
    {
        v[i].pos = VGet(p[i].x, groundY, p[i].z);
        v[i].dif = GetColorU8(255, 50, 50, 150);
        v[i].norm = VGet(0.0f, 1.0f, 0.0f);
    }
    WORD indices[6] = { 0, 1, 2, 0, 2, 3 };
    DrawPolygonIndexed3D(v, 4, indices, 2, DX_NONE_GRAPH, TRUE);

    // 2. 進行ゲージ（手前から奥へ伸びる濃い赤）
    if (progress > 0.001f)
    {
        VECTOR pProgress[4];
        pProgress[0] = p[0];
        pProgress[1] = p[1];
        pProgress[2] = VAdd(p[1], VScale(dir_, radius_ * progress));
        pProgress[3] = VAdd(p[0], VScale(dir_, radius_ * progress));

        VERTEX3D vp[4];
        for (int i = 0; i < 4; ++i)
        {
            vp[i].pos = VGet(pProgress[i].x, groundY + 0.05f, pProgress[i].z);
            vp[i].dif = GetColorU8(255, 60, 60, 225);
            vp[i].norm = VGet(0.0f, 1.0f, 0.0f);
        }
        DrawPolygonIndexed3D(vp, 4, indices, 2, DX_NONE_GRAPH, TRUE);
    }

    // 外枠ライン
    //unsigned int outlineColor = GetColor(255, 120, 120);
    //for (int i = 0; i < 4; ++i)
    //{
    //    DrawLine3D(v[i].pos, v[(i + 1) % 4].pos, outlineColor);
    //}

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    SetWriteZBuffer3D(TRUE);
}
