#pragma once
#include "AoEBase.h"
#include <cmath>
class AoEBox :
    public AoEBase
{
    public:
    AoEBox(const VECTOR& pos, float radius, float width, const VECTOR& dir, float duration)
        : AoEBase(pos, duration), radius_(radius), width_(width)
    {
		VECTOR d = VGet(dir.x, 0.0f, dir.z);
		dir_ = VNorm(d);
    }
    bool CheckHit(const VECTOR& playerPos) const override
    {
        // AoEの方向ベクトルを正規化
        VECTOR dirNorm = VNorm(dir_);
        // AoEの右方向ベクトルを計算（Y軸上方向を基準に）
        VECTOR right = VCross(VGet(0.0f, 1.0f, 0.0f), dirNorm);
        right = VNorm(right);
        // プレイヤー位置をAoE座標系に変換
        VECTOR toPlayer = VSub(playerPos, pos_);
        float forwardDist = VDot(toPlayer, dirNorm);
        float rightDist = VDot(toPlayer, right);
        return (forwardDist >= 0.0f && forwardDist <= radius_) &&
            (fabs(rightDist) <= width_ * 0.5f);
    }
	void Draw() const override;
private:
	float radius_;
    float width_;
    VECTOR dir_;
};

