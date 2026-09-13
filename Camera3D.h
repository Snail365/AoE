#pragma once
#include <DxLib.h>

class Camera3D
{
public:
    Camera3D();

    void Update(const VECTOR& targetPos);

    // 画面入力をクォータービュー用のワールド移動ベクトルへ変換
    VECTOR ConvertInputToWorldVector(float inputX, float inputZ) const;

private:
    VECTOR targetPos_ = VGet(0.0f, 0.0f, 0.0f);
};