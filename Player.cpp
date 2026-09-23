#include "Player.h"

Player::Player(FileManager& fileManager)
	: fileMng_(fileManager)
{
	Initialize();
}

Player::~Player()
{
}

void Player::Initialize()
{
	isDodging_ = false;
	dodgeTimer_ = 0.0f;
	isCounterReady_ = false;

	data_.id = 0;
	data_.pos = VGet(0.0f, 0.0f, 0.0f);
	data_.speed = 240.0f;
	data_.attackPower = 10;
	data_.hp = 100;
	data_.maxHp = 100;
	data_.isAlive = true;
	data_.chips = 0;
	data_.level = 1;
	data_.exp = 0;
	data_.expToNextLevel = 100;

	imageFile_ = nullptr;
	imageFile_ = fileMng_.LoadImageFM("Resource/Player/Player.png");
}

void Player::Update(float deltaTime, const VECTOR& moveDir)
{
	if (!data_.isAlive) return;

	if (data_.hp >= data_.maxHp)
	{
		data_.hp = data_.maxHp;
	}

	if (isDodging_)
	{
		dodgeTimer_ += deltaTime;

		data_.pos.x += dodgeDir_.x * (data_.speed * 1.8f) * deltaTime;
		data_.pos.z += dodgeDir_.z * (data_.speed * 1.8f) * deltaTime;

		if (dodgeTimer_ >= dodgeDuration_)
		{
			isDodging_ = false;
		}
		return; // 回避中は通常の移動入力を受け付けない
	}

	data_.pos.x += moveDir.x * data_.speed * deltaTime;
	data_.pos.z += moveDir.z * data_.speed * deltaTime;

	if (moveDir.x != 0.0f || moveDir.z != 0.0f)
	{
		facingDir_ = moveDir;
	}
}

void Player::Draw3D() const
{
	if (!data_.isAlive) return;

	if (imageFile_ != nullptr)
	{
		// 浮き上がり防止のため、足元基準で表示位置を底上げ
		VECTOR drawPos = VGet(data_.pos.x, data_.pos.y + size_ * 0.5f, data_.pos.z);

		// 常にカメラを向くビルボード描画
		DrawBillboard3D(drawPos, 0.5f, 0.5f, size_, 0.0f, imageFile_->GetHandle(), TRUE);
	}
	else
	{
		// 画像読み込み失敗時のフォールバック球体
		DrawSphere3D(VGet(data_.pos.x, data_.pos.y + 15.0f, data_.pos.z), 15.0f, 16, GetColor(100, 200, 255), GetColor(255, 255, 255), TRUE);
	}
}

void Player::ReSpawn()
{
	data_.hp = data_.maxHp;
	data_.isAlive = true;
	data_.pos = VGet(0.0f, 0.0f, 0.0f);
	isDodging_ = false;
	isCounterReady_ = false;
}

bool Player::BuyAndEquipWeapon(WeaponType type)
{
	if (HasWeapon(type))
	{
		currentWeapon_ = type;
		printfDx("既に持っています");
		return true;
	}

	int price = 0;
	if (type == WeaponType::Shotgun) price = 200;
	else if (type == WeaponType::Rifle) price = 500;

	if (data_.chips >= price)
	{
		data_.chips -= price;
		if (type == WeaponType::Shotgun) hasShotgun_ = true;
		else if (type == WeaponType::Rifle) hasRifle_ = true;
		currentWeapon_ = type;
		printfDx("武器を購入しました");
		return true;
	}
	else
	{
		printfDx("チップが足りません");
	}
	return false;
}

bool Player::HasWeapon(WeaponType type) const
{
	if (type == WeaponType::Handgun) return hasHandgun_;
	if (type == WeaponType::Shotgun) return hasShotgun_;
	if (type == WeaponType::Rifle) return hasRifle_;
	return false;
}

void Player::AddExp(int amount)
{
	data_.exp += amount;
	// レベルアップ判定
	while (data_.exp >= data_.expToNextLevel)
	{
		data_.exp -= data_.expToNextLevel;
		data_.level++;
		data_.maxHp += 20;
		data_.hp = data_.maxHp;// レベルアップ時にHPを全回復
		data_.attackPower += 5; // 攻撃力を増加
		data_.expToNextLevel = static_cast<int>(data_.expToNextLevel * 1.5f); // 次のレベルまでの経験値を増加
	}
}

void Player::AddChips(int amount)
{
	data_.chips += amount;
}

void Player::TakeDamage(int damage)
{
	if (!data_.isAlive) return;
	data_.hp = (data_.hp > damage) ? data_.hp - damage : 0;
	if (data_.hp == 0) data_.isAlive = false;
}

void Player::UpgradeAttackPower(int cost, int amount)
{
	if (data_.chips >= cost)
	{
		data_.chips -= cost;
		data_.attackPower += amount;
	}
}

void Player::UpgradeMaxHp(int cost, int amount)
{
	if (data_.chips >= cost)
	{
		data_.chips -= cost;
		data_.maxHp += amount;
		data_.hp = data_.maxHp; // HPを全回復
	}
}

void Player::StartDodge()
{
	if (isDodging_ || !data_.isAlive) return;

	isDodging_ = true;
	dodgeTimer_ = 0.0f;

	dodgeDir_ = (VSize(facingDir_) > 0.001f) ? VNorm(facingDir_) : VGet(0.0f, 0.0f, 1.0f);
}

bool Player::ConsumeCounterReady()
{
	if (isCounterReady_)
	{
		isCounterReady_ = false;
		return true;
	}
	return false;
}
