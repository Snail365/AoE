#pragma once
#include <vector>
#include <DxLib.h>
#include <memory>
#include "EntityData.h"
#include "AoEBase.h"
#include "Player.h"
#include "Enemy.h"
#include "NavGrid.h"
#include "FileManager.h"

enum class MapType { SafeHub, DangerZone };

class GameSystemManager
{
public:
    GameSystemManager(FileManager& fileManager);
    ~GameSystemManager() = default;

    void Initialize();
    void SpawnAoE(std::unique_ptr<AoEBase> aoe);
    void Update(float deltaTime, const VECTOR& playerMoveDir, bool isAttacking, const VECTOR& attackDir);

    const Player& GetPlayer() const { return *playerPtr_; }
	Player& GetPlayer() { return *playerPtr_; }

	const std::vector<std::unique_ptr<Enemy>>& GetEnemies() const { return enemies_; }
	std::vector<std::unique_ptr<Enemy>>& GetEnemies() { return enemies_; }

	const NavGrid& GetNavGrid() const { return navGrid_; }
	NavGrid& GetNavGrid() { return navGrid_; }

    const std::vector<std::unique_ptr<AoEBase>>& GetAoEList() const { return aoeList_; }
	const std::vector<BulletData>& GetBullets() const { return bullets_; }
	MapType GetCurrentMapType() const { return currentMapType_; }
	void SwitchToMap(MapType mapType);

	bool IsGameOver() const { return isGameOver_; }
	void ProcessRespawn(); // ペナルティ適用 & 拠点復帰

	bool IsNearWeaponShop() const { return currentMapType_ == MapType::SafeHub && VSize(VSub(playerPtr_->GetPos(), weaponShopPos_)) <= 50.0f; }
	bool IsNearArmorShop() const { return currentMapType_ == MapType::SafeHub && VSize(VSub(playerPtr_->GetPos(), armorShopPos_)) <= 50.0f; }
	bool IsNearItemShop() const { return currentMapType_ == MapType::SafeHub && VSize(VSub(playerPtr_->GetPos(), itemShopPos_)) <= 50.0f; }
	bool IsNearPortal() const { return currentMapType_ == MapType::SafeHub && VSize(VSub(playerPtr_->GetPos(), portalPos_)) <= 50.0f; }

	int GetCurrentRoom() const { return currentRoom_; }
	int GetMaxRoom() const { return maxRoom_; }
	bool IsClear() const { return isClear_; }
	bool IsRoomCleared() const { return isRoomCleared_; }

	bool IsNearNextRoomDoor() const
	{
		return currentMapType_ == MapType::DangerZone && isRoomCleared_ && VSize(VSub(playerPtr_->GetPos(), nextDoorPos_)) <= 60.0f;
	}

	void ProceedToNextRoom();

private:
	void GenerateRoom(int roomIndex);

	std::unique_ptr<Player> playerPtr_;
    std::vector<std::unique_ptr<AoEBase>> aoeList_;
    std::vector<std::unique_ptr<Enemy>> enemies_;
	std::vector<BulletData> bullets_;

	NavGrid navGrid_;

	int currentRoom_ = 1;
	int maxRoom_ = 3;
	bool isClear_ = false;
	bool isRoomCleared_ = false;

	VECTOR nextDoorPos_ = VGet(0.0f, 0.0f, 250.0f);

    float playerAttackCooldown_;
	FileManager& fileMng_;

	MapType currentMapType_ = MapType::SafeHub;

	VECTOR weaponShopPos_ = VGet(-200.0f, 0.0f, 0.0f);
	VECTOR armorShopPos_ = VGet(200.0f, 0.0f, 0.0f);
	VECTOR itemShopPos_ = VGet(0.0f, 0.0f, 0.0f);

	VECTOR portalPos_ = VGet(0.0f, 0.0f, 400.0f);

	bool isGameOver_ = false;

};