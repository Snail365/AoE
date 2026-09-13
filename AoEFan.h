#pragma once
#include "AoEBase.h"
#include <cmath>
class AoEFan :
    public AoEBase
{
    public:
    AoEFan(const VECTOR& pos, float radius, float angle, const VECTOR& dir, float duration)
        : AoEBase(pos, duration), radius_(radius), angle_(angle)
    {
		VECTOR d = VGet(dir.x, 0.0f, dir.z);
		dir_ = VNorm(d);
    }
    bool CheckHit(const VECTOR& playerPos) const override
    {
        VECTOR rel = VSub(playerPos, pos_);
        float distSq = rel.x * rel.x + rel.z * rel.z;

        if (distSq > radius_ * radius_) return false;
        if (distSq < 0.0001f) return true; // プレイヤーがAoEの中心にいる場合はヒット

        // 角度判定(内積)
        VECTOR normRel = VScale(rel, 1.0f / sqrtf(distSq));
        float dot = VDot(normRel, VNorm(dir_));

        float cosHalfAngle = cosf(angle_ * 0.5f * (DX_PI_F / 180.0f));
        return dot >= cosHalfAngle;
    }
	void Draw() const override;
private:
    float radius_;
    float angle_;
	VECTOR dir_;
};

