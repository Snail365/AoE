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
	data_.id = 0;
	data_.pos = VGet(0.0f, 0.0f, 0.0f);
	data_.speed = 4.0f;
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

	data_.pos.x += moveDir.x * data_.speed;
	data_.pos.z += moveDir.z * data_.speed;
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
