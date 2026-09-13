#include "Camera3D.h"

Camera3D::Camera3D()
{
    SetupCamera_Ortho(2000.0f);
    // 平行投影（パースを切りクォータービューにする）
}

void Camera3D::Update(const VECTOR& targetPos)
{
    targetPos_ = targetPos;
    constexpr float distance = 300.0f;
    VECTOR cameraPos = VGet(
        targetPos_.x - distance,
        targetPos_.y + distance * 1.2f, // 見下ろし角度の調整
        targetPos_.z - distance
    );
    SetCameraPositionAndTarget_UpVecY(cameraPos, targetPos_);
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