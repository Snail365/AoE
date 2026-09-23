#pragma once
#include <DxLib.h>
#include <memory>
#include <vector>
#include <string>
#include "EntityData.h"
#include "BehaviorTree.h"
#include "NavGrid.h"
#include "FileManager.h"
#include "ImageFile.h"

class GameSystemManager;

// プレイヤーに構成を合わせる
class Enemy
{
public:
    Enemy(FileManager& fileManager, int id, const VECTOR& startPos, int hp, float attackInterval);
    ~Enemy() = default;

    void Initialize(int id, const VECTOR& startPos, int hp, float attackInterval);
    void Update(float deltaTime, const VECTOR& playerPos, const NavGrid& navGrid, GameSystemManager& systemMng);
    void Draw3D() const;
    void DrawDebug3D() const;

    // ダメージ処理
    void TakeDamage(int damage);

    // ゲッター
    const CharacterData& GetData() const { return data_; }
    CharacterData& GetData() { return data_; }
    const VECTOR& GetPos() const { return data_.pos; }
    bool IsAlive() const { return data_.isAlive; }
    const std::string& GetCurrentStateName() const { return currentStateName_; }

private:
    void BuildBehaviorTree();
    void MoveTo(const VECTOR& targetPos, float deltaTime);

private:
    CharacterData data_;
    FileManager& fileMng_;
    std::shared_ptr<ImageFile> imageFile_;
    float size_ = 64.0f; // 3D空間上の表示サイズ（Playerと同等）

    // AI（Behavior Tree）関連
    std::shared_ptr<BTNode> treeRoot_;
    std::vector<VECTOR> currentPath_; // A*で計算されたパス
    float pathUpdateTimer_ = 0.0f;

    // BT実行用のテンポラリ変数
    VECTOR currentTargetPos_ = VGet(0, 0, 0);
    float currentDeltaTime_ = 0.0f;

    // デバッグ表示用
    std::string currentStateName_ = "Idle";
};