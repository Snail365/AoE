#include "GameSystemManager.h"
#include "AoECircle.h"
#include "AoEBox.h"
#include "AoEFan.h"

GameSystemManager::GameSystemManager(FileManager& fileManager)
	: fileMng_(fileManager)
{
	Initialize();
}

void GameSystemManager::Initialize()
{
	playerPtr_ = std::make_unique<Player>(fileMng_);
	enemies_.clear();

	// テスト用の敵を追加
	CharacterData enemy;
	enemy.id = 1;
	enemy.pos = VGet(200.0f, 0.0f, 200.0f);
	enemy.hp = 100;
	enemy.maxHp = 100;
	enemy.attackInterval = 3.0f;
	enemies_.push_back(enemy);

	aoeList_.clear();
}

// AoEを生成してリストに追加
void GameSystemManager::SpawnAoE(std::unique_ptr<AoEBase> aoe)
{
	if (aoe) aoeList_.push_back(std::move(aoe));
}

void GameSystemManager::Update(float deltaTime, const VECTOR& playerMoveDir, bool isAttacking, const VECTOR& attackDir)
{
	// プレイヤーの移動処理
	playerPtr_->Update(deltaTime, playerMoveDir);

	// プレイヤーの攻撃処理（連射制御）
	if (playerAttackCooldown_ > 0.0f)
	{
		playerAttackCooldown_ -= deltaTime;
	}

	if (playerPtr_->IsAlive() && isAttacking && playerAttackCooldown_ <= 0.0f)
	{
		playerAttackCooldown_ = 0.2f; // 0.2秒間隔で連射可能

		// 照準方向（指定がなければ前方）へ簡易的な射撃判定
		VECTOR shotDir = attackDir;
		if (VSize(shotDir) < 0.001f) shotDir = VGet(0.0f, 0.0f, 1.0f);
		shotDir = VNorm(shotDir);

		BulletData bullet;
		bullet.pos = VGet(playerPtr_->GetPos().x, playerPtr_->GetPos().y + 15.0f, playerPtr_->GetPos().z);
		bullet.dir = shotDir;
		bullet.speed = 1200.0f;
		bullet.maxDistance = 500.0f;
		bullet.movedDistance = 0.0f;
		bullet.isAlive = true;
		bullets_.push_back(bullet);
	}

	// 弾丸の更新と敵への着弾判定
	for (auto it = bullets_.begin(); it != bullets_.end(); )
	{
		float moveStep = it->speed * deltaTime;
		it->pos = VAdd(it->pos, VScale(it->dir, moveStep));
		it->movedDistance += moveStep;

		bool hit = false;
		constexpr float BULLET_RADIUS = 15.0f;
		constexpr float ENEMY_RADIUS = 20.0f;

		// 敵とのヒットテスト
		for (auto& enemy : enemies_)
		{
			if (!enemy.isAlive) continue;

			VECTOR toEnemy = VSub(enemy.pos, it->pos);
			toEnemy.y = 0; // XZ平面判定

			if (VSize(toEnemy) <= (BULLET_RADIUS + ENEMY_RADIUS))
			{
				enemy.hp -= 25;
				if (enemy.hp <= 0)
				{
					enemy.hp = 0;
					enemy.isAlive = false;
					playerPtr_->AddExp(35);
				}
				hit = true;
				break;
			}
		}

		// 射程限界または着弾で消去
		if (hit || it->movedDistance >= it->maxDistance)
		{
			it = bullets_.erase(it);
		}
		else
		{
			++it;
		}
	}

	// 各敵の自動AoE攻撃処理
	if (playerPtr_->IsAlive())
	{
		for (auto& enemy : enemies_)
		{
			if (!enemy.isAlive) continue;

			enemy.attackTimer += deltaTime;
			if (enemy.attackTimer >= enemy.attackInterval)
			{
				enemy.attackTimer = 0.0f;
				// プレイヤーの位置へ円形AoEを発射
				SpawnAoE(std::make_unique<AoECircle>(playerPtr_->GetPos(), 100.0f, 2.5f));
			}
		}
	}

	// AoEのタイマー更新と終了判定
	for (auto it = aoeList_.begin(); it != aoeList_.end(); )
	{
		(*it)->Update(deltaTime);

		if ((*it)->IsFinished())
		{
			if (playerPtr_->IsAlive() && (*it)->CheckHit(playerPtr_->GetPos()))
			{
				playerPtr_->TakeDamage(35);
			}
			it = aoeList_.erase(it); // unique_ptr により自動破棄
		}
		else
		{
			++it;
		}
	}
}