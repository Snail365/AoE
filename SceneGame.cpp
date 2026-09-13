#include "SceneGame.h"
#include "InputManager.h"
#include "AoECircle.h"
#include "AoEBox.h"
#include "AoEFan.h"

SceneGame::SceneGame(FileManager& fileMng, SceneManager& sceneMng) 
	: SceneSuper(fileMng, sceneMng) ,
	renderMng_(fileMng),
	camera_(),
	systemMng_(fileMng)
{

}

SceneGame::~SceneGame()
{
}

void SceneGame::Update()
{
	auto& input = InputManager::GetInstance();

    float inputX = 0.0f;
	float inputZ = 0.0f;

    inputX = input.GetActionValue(ActionID::MoveH);
    inputZ = input.GetActionValue(ActionID::MoveV);

    // パッドが接続されていれば左スティックの入力値を加算 (L_Yは奥方向がマイナスの場合があるため反転調整)
    if (input.IsPadConnect())
    {
        inputX += input.GetPadAxisValue(PadAxis::Pad_L_X);
        inputZ -= input.GetPadAxisValue(PadAxis::Pad_L_Y);
    }

    // -1.0 〜 1.0 の範囲にクランプ
    if (inputX > 1.0f) inputX = 1.0f; if (inputX < -1.0f) inputX = -1.0f;
    if (inputZ > 1.0f) inputZ = 1.0f; if (inputZ < -1.0f) inputZ = -1.0f;

	VECTOR playerPos = systemMng_.GetPlayer().GetPos();

    if (input.IsKeyTriggered(KEY_INPUT_1))
    {
        // 円形: (pos, radius, duration)
        systemMng_.SpawnAoE(std::make_unique<AoECircle>(playerPos, 100.0f, 2.0f)); // Circle
    }
    if (input.IsKeyTriggered(KEY_INPUT_2))
    {
        // 直線型: (pos, radius, width, dir, duration)
        VECTOR origin = VGet(0, 0, 0);
        VECTOR dirToPlayer = VSub(playerPos, origin);
        systemMng_.SpawnAoE(std::make_unique<AoEBox>(origin, 400.0f, 80.0f, dirToPlayer, 2.0f)); // Box
    }
    if (input.IsKeyTriggered(KEY_INPUT_3))
    {
        // 扇型: (pos, radius, angle, dir, duration)
		VECTOR origin = VGet(0, 0, 0);
		VECTOR dirToPlayer = VSub(playerPos, origin);
        systemMng_.SpawnAoE(std::make_unique<AoEFan>(origin, 300.0f, 60.0f, dirToPlayer, 2.0f)); // Fan
    }

    // クォータービュー座標変換
    VECTOR moveDir = camera_.ConvertInputToWorldVector(inputX, inputZ);

    bool isAttacking = input.IsKeyPressed(KEY_INPUT_Z);

    // システム更新
    systemMng_.Update(1.0f / 60.0f, moveDir, isAttacking, moveDir);

    // カメラをプレイヤーの位置に追従
    camera_.Update(systemMng_.GetPlayer().GetPos());
}

void SceneGame::Draw()
{
    // 描画はすべて RenderManager へ委託
    renderMng_.Draw3D(
        systemMng_.GetPlayer(),
        systemMng_.GetEnemies(),
        systemMng_.GetAoEList(),
		systemMng_.GetBullets()
    );

    renderMng_.Draw2D();

    InputManager::GetInstance().DrawDebug(10, 100);
}

void SceneGame::TransitionIn(float t)
{
}

void SceneGame::TransitionOut(float t)
{
}
