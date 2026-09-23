#include "GameSystemManager.h"
#include "AoECircle.h"
#include "AoEBox.h"
#include "AoEFan.h"

GameSystemManager::GameSystemManager(FileManager& fileManager)
	: fileMng_(fileManager),
	navGrid_(20, 20, 40.0f, VGet(-400.0f, 0.0f, -400.0f))
{
	Initialize();
}

void GameSystemManager::Initialize()
{
	if (playerPtr_)
	{
		playerPtr_->Initialize();
	}
	else
	{
		playerPtr_ = std::make_unique<Player>(fileMng_);
	}
	enemies_.clear();
	bullets_.clear();
	aoeList_.clear();
	playerAttackCooldown_ = 0.0f;
}

// AoEを生成してリストに追加
void GameSystemManager::SpawnAoE(std::unique_ptr<AoEBase> aoe)
{
	if (aoe) aoeList_.push_back(std::move(aoe));
}

void GameSystemManager::Update(float deltaTime, const VECTOR& playerMoveDir, bool isAttacking, const VECTOR& attackDir)
{
	// プレイヤー死亡検知（DangerZone時）
	if (currentMapType_ == MapType::DangerZone && !playerPtr_->IsAlive())
	{
		isGameOver_ = true;
		return; // 死亡中は以降のゲームロジック更新を停止
	}

	// プレイヤーの移動処理
	playerPtr_->Update(deltaTime, playerMoveDir);

	if (currentMapType_ == MapType::SafeHub)
	{
		enemies_.clear();
		bullets_.clear();
		aoeList_.clear();
	}
	else if (currentMapType_ == MapType::DangerZone)
	{
		// プレイヤーの攻撃処理（連射制御）
		if (playerAttackCooldown_ > 0.0f)
		{
			playerAttackCooldown_ -= deltaTime;
		}

		if (playerPtr_->IsAlive() && isAttacking && playerAttackCooldown_ <= 0.0f)
		{
			WeaponInfo weaponInfo = playerPtr_->GetCurrentWeaponInfo();
			playerAttackCooldown_ = weaponInfo.cooldown; // 0.2秒間隔で連射可能

			VECTOR baseDir = VNorm(playerPtr_->GetFacingDir()); // プレイヤーの移動方向に基づいて弾丸を発射
			bool isCounterShot = playerPtr_->ConsumeCounterReady(); // カウンター弾チェック(デフラグ兼用)

			for (int i = 0; i < weaponInfo.bulletCount; i++)
			{
				VECTOR shotDir = baseDir;

				// 現在3発同時発射に対応しているが、のちのちより多くの弾に対応させる必要があるかも
				if (weaponInfo.bulletCount > 1)
				{
					float angleOffset = (i - 1) * 0.2f;
					shotDir.x = baseDir.x * cosf(angleOffset) - baseDir.z * sinf(angleOffset);
					shotDir.z = baseDir.x * sinf(angleOffset) + baseDir.z * cosf(angleOffset);
				}

				BulletData bullet;
				bullet.pos = VGet(playerPtr_->GetPos().x, playerPtr_->GetPos().y + 15.0f, playerPtr_->GetPos().z);
				bullet.dir = shotDir;
				bullet.speed = isCounterShot ? weaponInfo.speed * 1.5f : weaponInfo.speed;
				bullet.maxDistance = isCounterShot ? weaponInfo.maxDistance * 1.3f : weaponInfo.maxDistance;
				bullet.power = isCounterShot ? (weaponInfo.power * 2) : weaponInfo.power;
				bullet.movedDistance = 0.0f;
				bullet.isAlive = true;
				bullets_.push_back(bullet);
			}
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
				if (!enemy->IsAlive()) continue;

				VECTOR toEnemy = VSub(enemy->GetPos(), it->pos);
				toEnemy.y = 0; // XZ平面判定

				if (VSize(toEnemy) <= (BULLET_RADIUS + ENEMY_RADIUS))
				{
					enemy->TakeDamage(it->power + playerPtr_->GetData().attackPower);
					if (!enemy->IsAlive())
					{
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

		// 敵の BehaviorTree & A* 更新
		if (playerPtr_->IsAlive())
		{
			for (auto& enemy : enemies_)
			{
				enemy->Update(deltaTime, playerPtr_->GetPos(), navGrid_, *this);
			}
		}

		bool hasActiveEnemy = false;
		for (const auto& enemy : enemies_)
		{
			if (enemy->IsAlive()) { hasActiveEnemy = true; break; }
		}

		// 部屋の敵を全滅させたとき
		if (!hasActiveEnemy && !isRoomCleared_)
		{
			isRoomCleared_ = true;
			if (currentRoom_ == maxRoom_)
			{
				isClear_ = true;        // ダンジョン全体のクリア
				playerPtr_->AddChips(500); // クリア報酬
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
					if (playerPtr_->IsJustDodgeWindow())
					{
						playerPtr_->AddChips(100);
						playerPtr_->SetCounterReady(true);
					}
					else if (playerPtr_->IsDodging())
					{
						// 通常回避
					}
					else
					{
						playerPtr_->TakeDamage(35);
					}
				}
				it = aoeList_.erase(it); // unique_ptr により自動破棄
			}
			else
			{
				++it;
			}
		}
	}
}

void GameSystemManager::SwitchToMap(MapType mapType)
{
	currentMapType_ = mapType;
	playerPtr_->GetData().pos = VGet(0, 0, 0);
	bullets_.clear();
	aoeList_.clear();
	isGameOver_ = false;

	if (currentMapType_ == MapType::DangerZone)
	{
		currentRoom_ = 1;
		isClear_ = false;
		playerPtr_->GetData().pos = VGet(0.0f, 0.0f, -200.0f);
		GenerateRoom(currentRoom_);
	}
	else if (currentMapType_ == MapType::SafeHub)
	{
		playerPtr_->GetData().pos = VGet(0.0f, 0.0f, 0.0f);
		enemies_.clear();
	}
}

void GameSystemManager::ProcessRespawn()
{
	auto& data = playerPtr_->GetData();
	data.chips /= 2;

	playerPtr_->ReSpawn();
	SwitchToMap(MapType::SafeHub);
}

void GameSystemManager::ProceedToNextRoom()
{
	if (currentRoom_ < maxRoom_)
	{
		currentRoom_++;
		playerPtr_->GetData().pos = VGet(0.0f, 0.0f, -200.0f);
		GenerateRoom(currentRoom_);
	}
}

void GameSystemManager::GenerateRoom(int roomIndex)
{
	enemies_.clear();
	bullets_.clear();
	aoeList_.clear();
	isRoomCleared_ = false;

	// NavGrid のリセットと中央の壁（テスト用障害物）設定
	for (int x = 0; x < navGrid_.GetWidth(); ++x)
	{
		for (int z = 0; z < navGrid_.GetHeight(); ++z)
		{
			navGrid_.SetWalkable(x, z, true);
		}
	}
	// 中央付近（x=10, z=5〜14）を通過不可にセット
	for (int z = 5; z <= 14; ++z)
	{
		navGrid_.SetWalkable(10, z, false);
	}

	// 部屋に応じた敵生成(後々Jsonファイルに各レベルと部屋で分けて書いてもいいかも)
	int enemyCount = roomIndex * 2;
	for (int i = 0; i < enemyCount; i++)
	{
		float angle = (360.0f / enemyCount) * i * (3.14159f / 180.0f);
		VECTOR spawnPos = VGet(cosf(angle) * 150.0f, 0.0f, sinf(angle) * 150.0f);
		int hp = 60 + (roomIndex * 30);
		float attackInterval = 3.0f - (roomIndex * 0.3f);

		// ★ Enemy インスタンスを生成
		enemies_.push_back(std::make_unique<Enemy>(fileMng_, i + 1, spawnPos, hp, attackInterval));
	}
}
