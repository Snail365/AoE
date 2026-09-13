#pragma once
#include <DxLib.h>
#include <vector>
#include <memory>
#include "EntityData.h"
#include "AoEBase.h"
#include "Player.h"

class FileManager;


class RenderManager
{
public:
    RenderManager(FileManager& fileMng);
    void Draw3D(const Player& player, const std::vector<CharacterData>& enemies, const std::vector<std::unique_ptr<AoEBase>>& aoeList, const std::vector<BulletData>& bullets);
    ~RenderManager() = default;

    // 2D/UI描画
    void Draw2D();

private:
    void DrawGrid();

    FileManager& fileMng_;
};