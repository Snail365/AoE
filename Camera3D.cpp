#include "Camera3D.h"
#include <cmath>

Camera3D::Camera3D()
{
    SetupCamera_Ortho(2000.0f);
    // 平行投影（パースを切りクォータービューにする）
}

void Camera3D::Update(const VECTOR& targetPos, float deltaTime)
{
    if (!isInitialized_)
    {
        currentTargetPos_ = targetPos;
        isInitialized_ = true;
    }

    // 線形補間(e^-kdtでフレーム対応)
    float lerpFactor = 1.0f - expf(-followSpeed_ * deltaTime);
    VECTOR diff = VSub(targetPos, currentTargetPos_);
    currentTargetPos_ = VAdd(currentTargetPos_, VScale(diff, lerpFactor));

    VECTOR shakeOffset = VGet(0.0f, 0.0f, 0.0f);

    if (shakeTimer_ > 0.0f)
    {
        shakeTimer_ -= deltaTime;
        float progress = shakeTimer_ / shakeDuration_; // 収束
        float currentIntensity = shakeIntensity_ * progress;

        shakeOffset.x = ((rand() % 100) / 50.0f - 1.0f) * currentIntensity;
        shakeOffset.y = ((rand() % 100) / 50.0f - 1.0f) * currentIntensity;
        shakeOffset.z = ((rand() % 100) / 50.0f - 1.0f) * currentIntensity;
    }

    VECTOR finalTarget = VAdd(currentTargetPos_, shakeOffset);
    VECTOR eye = VAdd(finalTarget, offset_);

    SetCameraPositionAndTarget_UpVecY(eye, finalTarget);
}

void Camera3D::AddShake(float intensity, float duration)
{
    shakeIntensity_ = intensity;
    shakeDuration_ = duration;
    shakeTimer_ = duration;
}

VECTOR Camera3D::ConvertInputToWorldVector(float inputX, float inputZ) const
{
    if (inputX == 0.0f && inputZ == 0.0f) return VGet(0, 0, 0);

    constexpr float INV_SQRT2 = 0.70710678f;
    VECTOR worldDir;
    worldDir.x = (inputX + inputZ) * INV_SQRT2;
    worldDir.y = 0.0f;
    worldDir.z = (-inputX + inputZ) * INV_SQRT2;

    float len = VSize(worldDir);
    if (len > 0.001f)
    {
        worldDir.x /= len;
        worldDir.z /= len;
    }
    return worldDir;
}

void Camera3D::UpdateEffects(float deltaTime)
{
}

void Camera3D::CameraReset()
{
    isInitialized_ = false;
}
