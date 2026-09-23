#include "RenderManager.h"
#include "Application.h"
#include "FileManager.h"
#include "Player.h"
#include "Enemy.h"

RenderManager::RenderManager(FileManager& fileMng)
	: fileMng_(fileMng)
{
}

void RenderManager::Draw3D(const Player& player,
						   const std::vector<std::unique_ptr<Enemy>>& enemies,
						   const std::vector<std::unique_ptr<AoEBase>>& aoeList,
						   const std::vector<BulletData>& bullets)
{
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	// 3D床（グリッド）の描画
	DrawGrid();
	// プレイヤー描画
	player.Draw3D();
	// 敵描画
	for (const auto& enemy : enemies)
	{
		enemy->Draw3D();
	}
	// AoE描画
	for (const auto& aoe : aoeList)
	{
		if (aoe)
		{
			aoe->Draw();
		}
	}
	// 弾丸描画
	unsigned int bulletColor = GetColor(255, 80, 200);
	for (const auto& b : bullets)
	{
		if (!b.isAlive) continue;
		// 弾頭の小さな球体
		DrawSphere3D(b.pos, 4.0f, 8, bulletColor, GetColor(255, 255, 255), TRUE);
		// 進行方向の後方へ伸びるライン（トレーサー演出）
		VECTOR trailEnd = VSub(b.pos, VScale(b.dir, 50.0f));
		DrawLine3D(b.pos, trailEnd, bulletColor);
	}

	// 各円形マーカー描画用ヘルパー関数
	auto DrawCircleMarker = [](const VECTOR& pos, float radius, unsigned int color)
		{
			const int DIV = 32;
			VERTEX3D vertices[DIV + 1];
			WORD indices[DIV * 3];
			vertices[0].pos = VGet(pos.x, pos.y + 0.1f, pos.z);
			vertices[0].dif = GetColorU8((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 150);
			vertices[0].norm = VGet(0.0f, 1.0f, 0.0f);
			for (int i = 0; i < DIV; ++i)
			{
				float angle = DX_PI_F * 2.0f * i / DIV;
				vertices[i + 1].pos = VGet(pos.x + cosf(angle) * radius, pos.y + 0.1f, pos.z + sinf(angle) * radius);
				vertices[i + 1].dif = vertices[0].dif;
				vertices[i + 1].norm = VGet(0.0f, 1.0f, 0.0f);
			}
			for (int i = 0; i < DIV; ++i)
			{
				indices[i * 3 + 0] = 0;
				indices[i * 3 + 1] = i + 1;
				indices[i * 3 + 2] = (i == DIV - 1) ? 1 : (i + 2);
			}
			DrawPolygonIndexed3D(vertices, DIV + 1, indices, DIV, DX_NONE_GRAPH, TRUE);
		};

	// ショップ各種マーカー
	DrawCircleMarker(VGet(-200.0f, 0.0f, 0.0f), 50.0f, GetColor(255, 255, 80));  // 武器
	DrawCircleMarker(VGet(0.0f, 0.0f, 0.0f), 50.0f, GetColor(80, 255, 80));     // アイテム
	DrawCircleMarker(VGet(200.0f, 0.0f, 0.0f), 50.0f, GetColor(255, 80, 80));   // 防具
	DrawCircleMarker(VGet(0.0f, 0.0f, 400.0f), 50.0f, GetColor(80, 80, 255));	// ポータル

	// 部屋の奥の扉マーカー（ダンジョン用）
	DrawCircleMarker(VGet(0.0f, 0.0f, 250.0f), 60.0f, GetColor(100, 200, 255));

	SetUseZBuffer3D(FALSE);
}

void RenderManager::DrawHUD(const Player& player, MapType mapType, int currentRoom, int maxRoom, bool isClear)
{
	const auto& data = player.GetData();

	// HPバー
	int barX = 20, barY = 20;
	int barWidth = 200, barHeight = 20;
	float hpRate = (float)data.hp / (float)data.maxHp;

	DrawBox(barX, barY, barX + barWidth, barY + barHeight, GetColor(50, 50, 50), TRUE); // 背景
	DrawBox(barX, barY, barX + (int)(barWidth * hpRate), barY + barHeight, GetColor(255, 50, 50), TRUE); // 残量
	DrawBox(barX, barY, barX + barWidth, barY + barHeight, GetColor(255, 255, 255), FALSE); // 枠線
	DrawFormatString(barX + 10, barY + 2, GetColor(255, 255, 255), "HP: %d / %d", data.hp, data.maxHp);

	// ステータス
	WeaponInfo weapon = player.GetCurrentWeaponInfo();
	int statusX = 20, statusY = 50, weaponY = 70;
	DrawFormatString(statusX, statusY, GetColor(255, 255, 255), "Chips: %d | Level: %d", data.chips, data.level);
	DrawFormatString(statusX, weaponY, GetColor(255, 255, 0), "Weapon: %s", weapon.name);

	// マップ状態
	if (mapType == MapType::SafeHub)
	{
		DrawString(1100, 20, "[ SAFE HUB ]", GetColor(100, 255, 100));
	}
	else
	{
		DrawString(1100, 20, "[ DANGER ZONE ]", GetColor(255, 100, 100));
		if (isClear)
			DrawString(1100, 45, "STATUS: CLEARED", GetColor(255, 255, 0));
		else
			DrawFormatString(1100, 45, GetColor(255, 255, 255), "ROOM: %d / %d", currentRoom, maxRoom);
	}
}

void RenderManager::DrawInteractionPrompts(const GameSystemManager& sysMng)
{
	// 安全マップ時のガイド表示(後にRenderManagerに委託)
	if (sysMng.GetCurrentMapType() == MapType::SafeHub)
	{
		if (sysMng.IsNearWeaponShop())
			DrawString(400, 500, "Press 'E' to open Weapon Shop", GetColor(255, 255, 0));
		else if (sysMng.IsNearArmorShop())
			DrawString(400, 500, "Press 'E' to open Armor Shop", GetColor(255, 255, 0));
		else if (sysMng.IsNearItemShop())
			DrawString(400, 500, "Press 'E' to open Item Shop", GetColor(255, 255, 0));
		else if (sysMng.IsNearPortal())
			DrawString(400, 500, "Press 'E' to Enter Danger Zone", GetColor(255, 100, 100));
	}
	else if (sysMng.GetCurrentMapType() == MapType::DangerZone)
	{
		// 部屋クリア後、扉に近づいたときのガイドメッセージ
		if (sysMng.IsRoomCleared() && sysMng.IsNearNextRoomDoor())
		{
			if (sysMng.GetCurrentRoom() < sysMng.GetMaxRoom())
			{
				DrawString(400, 500, "Press 'E' to Proceed to Next Room", GetColor(100, 255, 255));
			}
			else
			{
				DrawString(400, 500, "Press 'E' to Return to Safe Hub", GetColor(255, 255, 0));
			}
		}
	}
}

void RenderManager::DrawShopUI(const GameSystemManager& sysMng)
{
	DrawBox(200, 100, 1080, 620, GetColor(0, 0, 0), TRUE);
	DrawBox(200, 100, 1080, 620, GetColor(255, 255, 255), FALSE);

	auto& player = sysMng.GetPlayer();

	if (sysMng.IsNearWeaponShop())
	{
		DrawString(230, 130, "=== WEAPON SHOP ===", GetColor(255, 255, 0));
		DrawFormatString(230, 250, GetColor(255, 255, 255), "Current Chips: %d", sysMng.GetPlayer().GetData().chips);

		// 武器表示用ヘルパー
		auto DrawWeaponOption = [&](int y, WeaponType type, const char* keyStr)
			{
				const auto& info = WeaponMaster::GetWeaponInfo(type);
				bool isEquipped = (player.GetCurrentWeaponType() == type);
				bool isOwned = player.HasWeapon(type);

				if (isEquipped)
					DrawFormatString(230, y, GetColor(100, 255, 100), "[%s] %s [Equipped]", keyStr, info.name);
				else if (isOwned)
					DrawFormatString(230, y, GetColor(200, 200, 200), "[%s] %s [Owned - Press to Equip]", keyStr, info.name);
				else
					DrawFormatString(230, y, GetColor(255, 255, 255), "[%s] %s - %d Chips", keyStr, info.name, info.price);
			};

		DrawWeaponOption(220, WeaponType::Handgun, "1");
		DrawWeaponOption(260, WeaponType::Shotgun, "2");
		DrawWeaponOption(300, WeaponType::Rifle, "3");
	}
	else if (sysMng.IsNearArmorShop())
	{
		DrawString(230, 130, "=== ARMOR SHOP ===", GetColor(255, 255, 0));
		DrawFormatString(230, 200, GetColor(255, 255, 255), "[1] Upgrade MaxHP (+20) - 100 Chips (Current MaxHP: %d)", player.GetData().maxHp);
	}
	else if (sysMng.IsNearItemShop())
	{
		DrawString(230, 130, "=== ITEM SHOP ===", GetColor(255, 255, 0));
		DrawFormatString(230, 200, GetColor(255, 255, 255), "[1] Buy Chips (+100) - Free Sample", player.GetData().chips);
	}
}

void RenderManager::DrawGameOverUI(int currentChips)
{
	// 半透明の黒背景
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(0, 0, Application::SCREEN_WID, Application::SCREEN_HIG, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// テキスト表示
	DrawString(520, 280, "G A M E   O V E R", GetColor(255, 50, 50));

	int penaltyLoss = currentChips / 2;
	DrawFormatString(450, 340, GetColor(255, 255, 255), "Penalty: Lost %d Chips (50%%)", penaltyLoss);
	DrawString(440, 420, "Press 'R' or 'E' to Return to Safe Hub", GetColor(255, 255, 0));
}

void RenderManager::DrawClearUI(const GameSystemManager& sysMng)
{
	if (!sysMng.IsClear()) return;

	DrawString(520, 200, "STAGE CLEAR!", GetColor(255, 255, 0));
	DrawString(470, 230, "Bonus: +500 Chips Granted!", GetColor(100, 255, 100));
}

void RenderManager::DrawPlayerDebugInfo(const Player& player, int x, int y)
{
	unsigned int color = GetColor(255, 255, 255);
	// デバッグ用のプレイヤー情報表示
	// プレイヤー位置表示
	DrawFormatString(x, y, color, "Player Pos: (%.2f, %.2f, %.2f)", player.GetPos().x, player.GetPos().y, player.GetPos().z);
	// プレイヤー向き表示
	DrawFormatString(x, y + 20, color, "Player Facing Dir: (%.2f, %.2f, %.2f)", player.GetFacingDir().x, player.GetFacingDir().y, player.GetFacingDir().z);
	// プレイヤーの回避状態表示
	DrawFormatString(x, y + 260, color, "IsDodging: %s", player.IsDodging() ? "true" : "false");
	DrawFormatString(x, y + 280, color, "IsCounterReady: %s", player.IsCounterReady() ? "true" : "false");
	// プレイヤーの各データ
	//float speed
	//int attackPower
	//int hp
	//int maxHp
	//bool isAlive
	//float attackTimer
	//float attackInterval
	//int chips
	//int level
	//int exp
	//int expToNextLevel
	DrawFormatString(x, y + 40, color, "Player Data: ...");
	DrawFormatString(x, y + 60, color, "speed: %f", player.GetData().speed);
	DrawFormatString(x, y + 80, color, "attackPower: %d", player.GetData().attackPower);
	DrawFormatString(x, y + 100, color, "hp/maxhp: %d/%d", player.GetData().hp, player.GetData().maxHp);
	DrawFormatString(x, y + 120, color, "isAlive: %s", player.GetData().isAlive ? "true" : "false");
	DrawFormatString(x, y + 140, color, "attackTimer: %f", player.GetData().attackTimer);
	DrawFormatString(x, y + 160, color, "attackInterval: %f", player.GetData().attackInterval);
	DrawFormatString(x, y + 180, color, "chips: %d", player.GetData().chips);
	DrawFormatString(x, y + 200, color, "level: %d", player.GetData().level);
	DrawFormatString(x, y + 220, color, "exp: %d", player.GetData().exp);
	DrawFormatString(x, y + 240, color, "expToNextLevel: %d", player.GetData().expToNextLevel);
}

void RenderManager::DrawGrid()
{
	constexpr float GRID_SIZE = 50.0f;
	constexpr int GRID_COUNT = 20;

	unsigned int gridColor = GetColor(60, 60, 80);
	unsigned int axisXColor = GetColor(255, 80, 80); // 赤（X軸）
	unsigned int axisZColor = GetColor(80, 80, 255); // 青（Z軸）

	float limit = GRID_COUNT * GRID_SIZE;

	for (int i = -GRID_COUNT; i <= GRID_COUNT; ++i)
	{
		float pos = i * GRID_SIZE;
		DrawLine3D(VGet(-limit, 0, pos), VGet(limit, 0, pos), (i == 0) ? axisZColor : gridColor);
		DrawLine3D(VGet(pos, 0, -limit), VGet(pos, 0, limit), (i == 0) ? axisXColor : gridColor);
	}
}