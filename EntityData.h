#pragma once
#include <DxLib.h>

// キャラクター等の純粋データ
struct CharacterData
{
    int id = 0;
    VECTOR pos = VGet(0.0f, 0.0f, 0.0f);
    float speed = 4.0f;
	int attackPower = 10;
    int hp = 100;
    int maxHp = 100;
    bool isAlive = true;

    float attackTimer = 0.0f;
    float attackInterval = 3.0f; // 個別の攻撃周期（秒）

	int chips = 0; // 所持チップ数
	int level = 1; // キャラクターのレベル
	int exp = 0; // 経験値(敵の場合はドロップ経験値)
	int expToNextLevel = 100; // 次のレベルまでの経験値
};

// 弾丸の純粋データ
struct BulletData
{
    VECTOR pos = VGet(0.0f, 0.0f, 0.0f);
    VECTOR dir = VGet(0.0f, 0.0f, 1.0f);
    float speed = 1500.0f;
    int power = 25;
    float maxDistance = 400.0f;
    float movedDistance = 0.0f;
    bool isAlive = true;
};