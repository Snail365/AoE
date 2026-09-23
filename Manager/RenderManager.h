#pragma once
#include <DxLib.h>
#include <vector>
#include <memory>

#include "FileManager.h"
#include "GameSystemManager.h"

class FileManager;


class RenderManager
{
public:
    RenderManager(FileManager& fileMng);
    ~RenderManager() = default;

    void Draw3D(const Player& player,
                const std::vector<std::unique_ptr<Enemy>>& enemies,
                const std::vector<std::unique_ptr<AoEBase>>& aoeList,
                const std::vector<BulletData>& bullets);

    // 2D/UI描画
    void DrawHUD(const Player& player, MapType mapType, int currentRoom, int maxRoom, bool isClear);
    void DrawInteractionPrompts(const GameSystemManager& sysMng);
    void DrawShopUI(const GameSystemManager& sysMng);
    void DrawGameOverUI(int currentChips);
    void DrawClearUI(const GameSystemManager& sysMng);
	void DrawPlayerDebugInfo(const Player& player, int x, int y);

private:
    void DrawGrid();

    FileManager& fileMng_;
};