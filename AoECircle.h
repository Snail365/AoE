#pragma once
#include "AoEBase.h"
#include <DxLib.h>
#include <cmath>
class AoECircle :
	public AoEBase
{
public:
	AoECircle(const VECTOR& pos, float radius, float duration)
		: AoEBase(pos, duration), radius_(radius)
	{
	}
	bool CheckHit(const VECTOR& playerPos) const override
	{
		float dx = playerPos.x - pos_.x;
		float dz = playerPos.z - pos_.z;
		return (dx * dx + dz * dz) <= (radius_ * radius_);
	}
	void Draw() const override;
private:
	float radius_;
};

