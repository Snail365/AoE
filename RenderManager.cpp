#include "RenderManager.h"
#include "FileManager.h"
#include "Player.h"

RenderManager::RenderManager(FileManager& fileMng)
	: fileMng_(fileMng)
{
}

void RenderManager::Draw3D(const Player& player, const std::vector<CharacterData>& enemies, const std::vector<std::unique_ptr<AoEBase>>& aoeList, const
  std::vector<BulletData>& bullets)
{
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	// 3D床（グリッド）の描画
	DrawGrid();

	// タイプ別のAoE描画
	for (const auto& aoe : aoeList)
	{
		if (aoe)
		{
			aoe->Draw();
		}
	}

	// 2. 弾丸描画（ピンクの軌跡レーザー）
	unsigned int bulletColor = GetColor(255, 80, 200);
	for (const auto& b : bullets)
	{
		if (!b.isAlive) continue;

		// 弾頭の小さな球体
		DrawSphere3D(b.pos, 4.0f, 8, bulletColor, GetColor(255, 255, 255), TRUE);

		// 進行方向の後方へ伸びるライン（トレーサー演出）
		VECTOR trailEnd = VSub(b.pos, VScale(b.dir, 25.0f));
		DrawLine3D(b.pos, trailEnd, bulletColor);
	}

	// 敵キャラ描画（赤色球体）
	for (const auto& enemy : enemies)
	{
		if (enemy.isAlive)
		{
			DrawSphere3D(VGet(enemy.pos.x, enemy.pos.y + 20.0f, enemy.pos.z), 20.0f, 16, GetColor(255, 80, 80), GetColor(255, 255, 255), TRUE);
		}
	}

	// 2. プレイヤー位置の簡易表示（デバッグ用球体）
	player.Draw3D();

	SetUseZBuffer3D(FALSE);
}

void RenderManager::Draw2D()
{
	//DrawString(10, 10, "Step 2: Quarter View Grid & RenderManager Setup", GetColor(255, 255, 255));
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