#pragma once
#include <DxLib.h>

class AoEBase
{
public:
    AoEBase(const VECTOR& pos, float duration)
        : pos_(pos), duration_(duration), timer_(0.0f)
    {
    }
    virtual ~AoEBase() = default;

    virtual void Update(float deltaTime) { timer_ += deltaTime; }
    virtual bool IsFinished() const { return timer_ >= duration_; }

    // ポリモーフィズムで呼び出す純粋仮想関数
    virtual bool CheckHit(const VECTOR& playerPos) const = 0;
    virtual void Draw() const = 0;

protected:
    VECTOR pos_;
    float duration_;
    float timer_;

    float GetProgress() const
    {
        float p = timer_ / duration_;
        return (p > 1.0f) ? 1.0f : p;
    }
};