#pragma once
#include <unordered_map>

enum class WeaponType
{
	Handgun,
	Shotgun,
	Rifle,
};

struct WeaponInfo
{
	WeaponType type;
	const char* name;
	int price;
	float cooldown;    // 連射速度
	float speed;       // 弾速
	float maxDistance; // 射程
	int power;       // 1発あたりの攻撃力
	int bulletCount;   // 1回の射撃で出る弾数
};

class WeaponMaster
{
public:
	static const WeaponInfo& GetWeaponInfo(WeaponType type)
	{

		static const std::unordered_map<WeaponType, WeaponInfo> masterTable =
		{
			{ WeaponType::Handgun, { WeaponType::Handgun, "Handgun",   0, 0.20f, 1200.0f, 500.0f, 25, 1 } },
			{ WeaponType::Shotgun, { WeaponType::Shotgun, "Shotgun", 300, 0.50f, 1000.0f, 400.0f, 20, 3 } },
			{ WeaponType::Rifle,   { WeaponType::Rifle,   "Rifle",   600, 0.12f, 2000.0f, 900.0f, 45, 1 } }
		};
		return masterTable.at(type);
	}
};