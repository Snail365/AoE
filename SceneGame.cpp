#include "SceneGame.h"
#include "InputManager.h"
#include "AoECircle.h"
#include "AoEBox.h"
#include "AoEFan.h"
#include "ImGuiManager.h"

SceneGame::SceneGame(FileManager& fileMng, SceneManager& sceneMng) 
	: SceneSuper(fileMng, sceneMng) ,
	camera_()
{
    renderMng_ = std::make_unique<RenderManager>(fileMng);
    systemMng_ = std::make_unique<GameSystemManager>(fileMng);
}

SceneGame::~SceneGame()
{
}

void SceneGame::Update()
{
	auto& input = InputManager::GetInstance();

    if (systemMng_->IsGameOver())
    {
        if (input.IsKeyReleased(KEY_INPUT_E) || input.IsPadTriggered(0, PadButton::Pad_Face_Down))
        {
            systemMng_->ProcessRespawn();
        }

        // カメラの更新は続ける
        camera_.Update(systemMng_->GetPlayer().GetPos(), 1.0f / 60.0f);
        return;
    }

    float inputX = 0.0f;
    float inputZ = 0.0f;
	bool isAttacking = false;

    if (isShopOpen_)
    {
        auto& player = systemMng_->GetPlayer();

        // ショップ入力
        if (systemMng_->IsNearWeaponShop())
        {
            if (input.IsKeyReleased(KEY_INPUT_1)) player.BuyAndEquipWeapon(WeaponType::Handgun);
            if (input.IsKeyReleased(KEY_INPUT_2)) player.BuyAndEquipWeapon(WeaponType::Shotgun);
            if (input.IsKeyReleased(KEY_INPUT_3)) player.BuyAndEquipWeapon(WeaponType::Rifle);
        }
        else if (systemMng_->IsNearArmorShop())
        {
            if (input.IsKeyReleased(KEY_INPUT_1)) player.UpgradeMaxHp(100, 20);      // HP強化
        }
        else if (systemMng_->IsNearItemShop())
        {
            if (input.IsKeyReleased(KEY_INPUT_1)) player.AddChips(100);               // チップ購入
        }

        if (input.IsKeyReleased(KEY_INPUT_ESCAPE) || input.IsKeyReleased(KEY_INPUT_E) || input.IsPadTriggered(0, PadButton::Xbox_Back))
        {
            isShopOpen_ = false;
        }
        // パッド実装に合わせて後々閉じるボタンUIが必要
    }
    else
    {
		// ショップやポータルの近くでEキーが押された場合の処理-------------------------------------------------------------------
        if (input.IsKeyReleased(KEY_INPUT_E) || input.IsPadTriggered(0, PadButton::Pad_Face_Down))
        {
            if (systemMng_->GetCurrentMapType() == MapType::SafeHub)
            {
                if (systemMng_->IsNearPortal())
                {
                    systemMng_->SwitchToMap(MapType::DangerZone);
                    camera_.CameraReset();
                    return;
                }
                else if (systemMng_->IsNearWeaponShop() || systemMng_->IsNearArmorShop() || systemMng_->IsNearItemShop())
                {
                    isShopOpen_ = true;
                }
            }
            else if (systemMng_->GetCurrentMapType() == MapType::DangerZone)
            {
                if (systemMng_->IsRoomCleared() && systemMng_->IsNearNextRoomDoor())
                {
                    if (systemMng_->GetCurrentRoom() < systemMng_->GetMaxRoom())
                    {
                        systemMng_->ProceedToNextRoom(); // 次の部屋へ進む
                        camera_.CameraReset();
                        return;
                    }
                    else
                    {
                        systemMng_->SwitchToMap(MapType::SafeHub); // 全クリアして拠点へ帰還
                        camera_.CameraReset();
                        return;
                    }
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        inputX = input.GetActionValue(ActionID::MoveH);
        inputZ = input.GetActionValue(ActionID::MoveV);

        // パッドが接続されていれば左スティックの入力値を加算 (L_Yは奥方向がマイナスの場合があるため反転調整)
        if (input.IsPadConnect())
        {
            inputX += input.GetPadAxisValue(PadAxis::Pad_L_X);
            inputZ += input.GetPadAxisValue(PadAxis::Pad_L_Y);
        }

        // -1.0 〜 1.0 の範囲にクランプ
        if (inputX > 1.0f) inputX = 1.0f; if (inputX < -1.0f) inputX = -1.0f;
        if (inputZ > 1.0f) inputZ = 1.0f; if (inputZ < -1.0f) inputZ = -1.0f;

        // 回避入力のチェック
        if (input.IsKeyTriggered(KEY_INPUT_LSHIFT) || input.IsPadTriggered(0, PadButton::Pad_Face_Right))
        {
            systemMng_->GetPlayer().StartDodge();
            //camera_.AddShake(8.0f, 0.5f);
        }
        isAttacking = input.IsKeyPressed(KEY_INPUT_SPACE) || input.IsPadPressed(0, PadButton::Pad_R_Trigger);
    }

    // クォータービュー座標変換
    VECTOR moveDir = camera_.ConvertInputToWorldVector(inputX, inputZ);
    // システム更新
    systemMng_->Update(1.0f / 60.0f, moveDir, isAttacking, moveDir);
    // カメラをプレイヤーの位置に追従
    camera_.Update(systemMng_->GetPlayer().GetPos(), 1.0f / 60.0f);

}

void SceneGame::Draw()
{
    // 描画はすべて RenderManager へ委託
    renderMng_->Draw3D(
        systemMng_->GetPlayer(),
        systemMng_->GetEnemies(),
        systemMng_->GetAoEList(),
		systemMng_->GetBullets()
    );

    renderMng_->DrawHUD(
        systemMng_->GetPlayer(),
        systemMng_->GetCurrentMapType(),
        systemMng_->GetCurrentRoom(),
        systemMng_->GetMaxRoom(),
        systemMng_->IsClear());
    
    if (systemMng_->IsGameOver())
    {
        renderMng_->DrawGameOverUI(systemMng_->GetPlayer().GetData().chips);
    }
    // ショップ画面の表示
    if (isShopOpen_)
    {
        renderMng_->DrawShopUI(*systemMng_);
    }
    else
    {
        renderMng_->DrawInteractionPrompts(*systemMng_);
        renderMng_->DrawClearUI(*systemMng_);
    }

    // 入力デバッグ情報
    InputManager::GetInstance().DrawDebug(800, 20);

    DebugUIDraw();
    // NavGrid のデバッグ描画（ImGuiフラグがONの時のみ3D空間に描画）
    if (showNavMesh_)
    {
        SetUseZBuffer3D(TRUE);
        SetWriteZBuffer3D(TRUE);
        
        systemMng_->GetNavGrid().DrawDebugGrid({});

        for (const auto& enemy : systemMng_->GetEnemies())
        {
            enemy->DrawDebug3D();
        }
        SetUseZBuffer3D(FALSE);
    }
}

void SceneGame::TransitionIn(float t)
{
}

void SceneGame::TransitionOut(float t)
{
}

void SceneGame::DebugUIDraw()
{
    // ImGuiデバッグ表示
    ImGuiManager::GetInstance().NewFrame();
    // デバッグウィンドウの作成
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Game Debugger * Editor");

    if (ImGui::BeginTabBar("DebugTabs"))
    {
        // ------------------------------------------------------------
        // タブ1: プレイヤーステータス & 武器調整
        // ------------------------------------------------------------
        if (ImGui::BeginTabItem("Player & Weapon"))
        {
            auto& playerData = systemMng_->GetPlayer().GetData();

            ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "[ Player Stats ]");
            ImGui::DragInt("Max HP", &playerData.maxHp, 1.0f, 1, 10000);
            ImGui::DragInt("Current HP", &playerData.hp, 1.0f, 0, playerData.maxHp);
            ImGui::SliderFloat("Move Speed", &playerData.speed, 50.0f, 1000.0f);
            ImGui::DragInt("Chips", &playerData.chips, 10.0f, 0, 999999);

            if (ImGui::Button("Full Heal"))
            {
                playerData.hp = playerData.maxHp;
            }

            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[ Current Weapon Info ]");

            auto currentType = systemMng_->GetPlayer().GetCurrentWeaponType();
            auto info = systemMng_->GetPlayer().GetCurrentWeaponInfo();
            ImGui::Text("Equipped: %s", info.name);
            ImGui::Text("Base Damage: %d", info.power);
            ImGui::Text("Attack Rate: %.2f sec", info.cooldown);

            if (ImGui::Button("Switch to Handgun")) systemMng_->GetPlayer().BuyAndEquipWeapon(WeaponType::Handgun);
            ImGui::SameLine();
            if (ImGui::Button("Switch to Shotgun")) systemMng_->GetPlayer().BuyAndEquipWeapon(WeaponType::Shotgun);
            ImGui::SameLine();
            if (ImGui::Button("Switch to Rifle"))   systemMng_->GetPlayer().BuyAndEquipWeapon(WeaponType::Rifle);

            ImGui::EndTabItem();
        }

        // ------------------------------------------------------------
        // タブ2: 敵＆AoEスポナー（リアルタイムテスト）
        // ------------------------------------------------------------
        if (ImGui::BeginTabItem("Spawner Test"))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[ Instant AoE Trigger ]");

            VECTOR playerPos = systemMng_->GetPlayer().GetPos();

            if (ImGui::Button("Spawn Circle AoE (At Player)"))
            {
                systemMng_->SpawnAoE(std::make_unique<AoECircle>(playerPos, 120.0f, 1.5f));
            }
            if (ImGui::Button("Spawn Box AoE (Forward)"))
            {
                systemMng_->SpawnAoE(std::make_unique<AoEBox>(VGet(0, 0, 0), 400.0f, 80.0f, VSub(playerPos, VGet(0, 0, 0)), 2.0f));
            }
            if (ImGui::Button("Spawn Fan AoE (Forward)"))
            {
                systemMng_->SpawnAoE(std::make_unique<AoEFan>(VGet(0, 0, 0), 300.0f, 60.0f, VSub(playerPos, VGet(0, 0, 0)), 2.0f));
            }

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "[ Active Entities Info ]");
            ImGui::Text("Active Enemies: %zu", systemMng_->GetEnemies().size());
            ImGui::Text("Active Bullets: %zu", systemMng_->GetBullets().size());
            ImGui::Text("Active AoEs:    %zu", systemMng_->GetAoEList().size());

            ImGui::EndTabItem();
        }

        // ------------------------------------------------------------
        // タブ3: カメラ ＆ 演出（Juice）
        // ------------------------------------------------------------
        if (ImGui::BeginTabItem("Camera & Effects"))
        {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "[ Camera Shake Test ]");

            static float testIntensity = 15.0f;
            static float testDuration = 0.3f;

            ImGui::SliderFloat("Shake Intensity", &testIntensity, 1.0f, 100.0f);
            ImGui::SliderFloat("Shake Duration", &testDuration, 0.05f, 10.0f);

            if (ImGui::Button("Trigger Shake Test"))
            {
                camera_.AddShake(testIntensity, testDuration);
            }

            ImGui::EndTabItem();
        }

        // ------------------------------------------------------------
        // タブ4: 視覚デバッグ設定（AI・判定可視化用）
        // ------------------------------------------------------------
        if (ImGui::BeginTabItem("Visual Debug"))
        {
            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.2f, 1.0f), "[ Visualization Toggles ]");
            ImGui::Checkbox("Show A* NavGrid & Enemy Paths", &showNavMesh_);

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "[ Enemy BT & AI Status ]");

            int index = 0;
            for (const auto& enemy : systemMng_->GetEnemies())
            {
                if (!enemy->IsAlive()) continue;
                ImGui::Text("Enemy #%d | HP: %d/%d | State: %s",
                            enemy->GetData().id,
                            enemy->GetData().hp,
                            enemy->GetData().maxHp,
                            enemy->GetCurrentStateName().c_str()
                );
                index++;
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    // ImGui描画実行
    ImGuiManager::GetInstance().Render();
}
