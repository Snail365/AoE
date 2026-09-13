#pragma once
#include <vector>
#include <DxLib.h>
#include <memory>
#include "EntityData.h"
#include "AoEBase.h"
#include "Player.h"
#include "FileManager.h"

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
    const std::vector<CharacterData>& GetEnemies() const { return enemies_; }
    const std::vector<std::unique_ptr<AoEBase>>& GetAoEList() const { return aoeList_; }
	const std::vector<BulletData>& GetBullets() const { return bullets_; }

private:
	std::unique_ptr<Player> playerPtr_;
    std::vector<std::unique_ptr<AoEBase>> aoeList_;
    std::vector<CharacterData> enemies_;
	std::vector<BulletData> bullets_;

    float playerAttackCooldown_;
	FileManager& fileMng_;
};