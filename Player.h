#pragma once
#include <DxLib.h>
#include "EntityData.h"
#include "FileManager.h"
#include "ImageFile.h"
#include "WeaponData.h"

#include <memory>

class Player
{
public:
    Player(FileManager& fileManager);
    ~Player();

    void Initialize();
    void Update(float deltaTime, const VECTOR& moveDir);
    void Draw3D() const;

    void ReSpawn();

	// 武器操作
	bool BuyAndEquipWeapon(WeaponType type);
	WeaponType GetCurrentWeaponType() const { return currentWeapon_; }
    WeaponInfo GetCurrentWeaponInfo() const { return WeaponMaster::GetWeaponInfo(currentWeapon_); }
	bool HasWeapon(WeaponType type) const;

    // ステータス操作
    void AddExp(int amount);
    void AddChips(int amount);
    void TakeDamage(int damage);

    void UpgradeAttackPower(int cost, int amount);
    void UpgradeMaxHp(int cost, int amount);

    // ゲッター
    const CharacterData& GetData() const { return data_; }
    CharacterData& GetData() { return data_; }
    const VECTOR& GetPos() const { return data_.pos; }
	const VECTOR& GetFacingDir() const { return facingDir_; }
    bool IsAlive() const { return data_.isAlive; }

    void StartDodge();
    bool IsDodging() const { return isDodging_; }
    bool IsJustDodgeWindow() const { return isDodging_ && (dodgeTimer_ <= justDodgeWindow_); }

    // カウンター状態
    bool IsCounterReady() const { return isCounterReady_; }
    void SetCounterReady(bool ready) { isCounterReady_ = ready; }
    bool ConsumeCounterReady(); // カウンター攻撃発動時に消費

private:
    CharacterData data_;
	std::shared_ptr<ImageFile> imageFile_;
    float size_ = 64.0f; // 3D空間上の表示サイズ
	FileManager& fileMng_;
	VECTOR facingDir_ = VGet(0.0f, 0.0f, 1.0f); // プレイヤーの向き

	// 現在の武器タイプ
	WeaponType currentWeapon_ = WeaponType::Handgun;
	// 武器所持フラグ(既に購入しているか)
    bool hasHandgun_ = true;
	bool hasShotgun_ = false;
	bool hasRifle_ = false;

    bool isDodging_;
    float dodgeTimer_;
    float dodgeDuration_ = 0.4f;      // 回避全体の時間
    float justDodgeWindow_ = 0.15f;   // 回避開始直後のジャスト回避受付時間
    VECTOR dodgeDir_ = VGet(0, 0, 0); // 回避方向

    bool isCounterReady_;     // カウンター準備完了フラグ
};