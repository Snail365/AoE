#pragma once
#include <DxLib.h>
#include "EntityData.h"

#include "FileManager.h"
#include "ImageFile.h"
#include <memory>

class Player
{
public:
    Player(FileManager& fileManager);
    ~Player();

    void Initialize();
    void Update(float deltaTime, const VECTOR& moveDir);
    void Draw3D() const;

    // ステータス操作
    void AddExp(int amount);
    void AddChips(int amount);
    void TakeDamage(int damage);

    // ゲッター
    const CharacterData& GetData() const { return data_; }
    CharacterData& GetData() { return data_; }
    const VECTOR& GetPos() const { return data_.pos; }
    bool IsAlive() const { return data_.isAlive; }

private:
    CharacterData data_;
	std::shared_ptr<ImageFile> imageFile_;
    float size_ = 128.0f; // 3D空間上の表示サイズ
	FileManager& fileMng_;
};