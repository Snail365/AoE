#pragma once
#include <DxLib.h>

class Camera3D
{
public:
    Camera3D();

    void Update(const VECTOR& targetPos, float deltaTime);
    void AddShake(float intensity, float duration);

    // 画面入力をクォータービュー用のワールド移動ベクトルへ変換
    VECTOR ConvertInputToWorldVector(float inputX, float inputZ) const;
    void UpdateEffects(float deltaTime); // 様々なエフェクトのタイマーの処理
    
    void CameraReset();
private:
    VECTOR offset_ = VGet(-300.0f, 400.0f, -300.0f);
    VECTOR currentTargetPos_ = VGet(0.0f, 0.0f, 0.0f);

    bool isInitialized_ = false;

    float followSpeed_ = 5.0f;

    float shakeTimer_ = 0.0f;
    float shakeDuration_ = 0.0f;
    float shakeIntensity_ = 0.0f;
};