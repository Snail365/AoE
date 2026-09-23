#pragma once
#include <DxLib.h>
#include <vector>

// 1マスデータ
struct AStarNode
{
	int x = 0;
	int z = 0;
	bool isWalkable = true;

    // A*計算用スコア
    float gCost = 0.0f; // スタートからの実コスト
	float hCost = 0.0f; // ゴールまでの推定コスト
	float FCost() const { return gCost + hCost; }

	// 経路復元用座標変数
	int parentX = -1;
	int parentZ = -1;

    // しょきか
	void Reset()
	{
		gCost = 99999999.0f;
		hCost = 0.0f;
		parentX = -1;
		parentZ = -1;
	}
};
class NavGrid
{
public:
	NavGrid(int gridWidth, int gridHeight, float cellSize, VECTOR originPos);

    // 3Dワールド座標とグリッド座標の変換
    bool WorldToGrid(const VECTOR& worldPos, int& outX, int& outZ) const;
    VECTOR GridToWorld(int x, int z) const;

    // 障害物の設定・判定
    void SetWalkable(int x, int z, bool walkable);
    bool IsWalkable(int x, int z) const;

    // A* 経路探索のメイン関数
    std::vector<VECTOR> FindPath(const VECTOR& startWorld, const VECTOR& targetWorld) const;

    // デバッグ描画
    void DrawDebugGrid(const std::vector<VECTOR>& currentPath) const;

    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }

private:
    int width_;
    int height_;
    float cellSize_;
    VECTOR originPos_; // グリッドの左下（原点）のワールド座標

    std::vector<std::vector<AStarNode>> nodes_;

    // マンハッタン距離によるヒューリスティック計算
    float CalculateHeuristic(int x1, int z1, int x2, int z2) const;
};

