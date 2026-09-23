#include "NavGrid.h"
#include <cmath>
#include <algorithm>

NavGrid::NavGrid(int gridWidth, int gridHeight, float cellSize, VECTOR originPos)
	: width_(gridWidth),
	height_(gridHeight),
	cellSize_(cellSize),
	originPos_(originPos)
{
	nodes_.resize(width_, std::vector<AStarNode>(height_));

	for (int x = 0; x < width_; x++)
	{
		for (int z = 0; z < height_; z++)
		{
			nodes_[x][z].x = x;
			nodes_[x][z].z = z;
			nodes_[x][z].isWalkable = true;
		}
	}
}
// ワールド座標からグリッド座標へ
bool NavGrid::WorldToGrid(const VECTOR& worldPos, int& outX, int& outZ) const
{
	outX = static_cast<int>((worldPos.x - originPos_.x) / cellSize_);
	outZ = static_cast<int>((worldPos.z - originPos_.z) / cellSize_);

	return (outX >= 0 && outX < width_ && outZ >= 0 && outZ < height_);
}
// グリッド座標からワールド座標へ 
VECTOR NavGrid::GridToWorld(int x, int z) const
{
	return VGet(
		originPos_.x + (x + 0.5f) * cellSize_,
		originPos_.y,
		originPos_.z + (z + 0.5f) * cellSize_
	);
}
// 障害物設定
void NavGrid::SetWalkable(int x, int z, bool walkable)
{
	if (x >= 0 && x < width_ && z >= 0 && z < height_)
	{
		nodes_[x][z].isWalkable = walkable;
	}
}
// 障害物判定
bool NavGrid::IsWalkable(int x, int z) const
{
	if (x >= 0 && x < width_ && z >= 0 && z < height_)
	{
		return nodes_[x][z].isWalkable;
	}
	return false;
}
// マンハッタン距離によるヒューリスティック計算
float NavGrid::CalculateHeuristic(int x1, int z1, int x2, int z2) const
{
	return (std::abs(x1 - x2) + std::abs(z1 - z2)) * 10.0f;
}
// A*経路探索
std::vector<VECTOR> NavGrid::FindPath(const VECTOR& startWorld, const VECTOR& targetWorld) const
{
	std::vector<VECTOR> path;
	int startX, startZ, targetX, targetZ;

	// 例外処理
	// 判定と同時に各座標変数の初期化
	if (!WorldToGrid(startWorld, startX, startZ) || !WorldToGrid(targetWorld, targetX, targetZ))
	{
		return path; // 範囲外
	}
	if (!IsWalkable(targetX, targetZ))
	{
		return path; // ゴールが障害物の中
	}

	std::vector<std::vector<AStarNode>> searchNodes = nodes_;
	for (int x = 0; x < width_; x++)
	{
		for (int z = 0; z < height_; z++)
		{
			searchNodes[x][z].Reset();
		}
	}

	std::vector<AStarNode*> openSet; // 未探索リスト
	std::vector<AStarNode*> closedSet; // 探索済みリスト

	AStarNode* startNode = &searchNodes[startX][startZ];
	startNode->gCost = 0.0f;
	startNode->hCost = CalculateHeuristic(startX, startZ, targetX, targetZ);
	openSet.push_back(startNode);

	while (!openSet.empty())
	{
		// 最も FCost が低いノードを探す
		auto currentIt = std::min_element(openSet.begin(), openSet.end(), [](AStarNode* a, AStarNode* b)
											   {
												   return a->FCost() < b->FCost();
											   });
		AStarNode* current = *currentIt;
		// ゴール到達
		if (current->x == targetX && current->z == targetZ)
		{
			// 探索結果をプッシュ
			AStarNode* curr = current;
			while (curr != nullptr)
			{
				path.push_back(GridToWorld(curr->x, curr->z));
				if (curr->parentX == -1 || curr->parentZ == -1) break;
				curr = &searchNodes[curr->parentX][curr->parentZ];
			}
			// 逆向きのためリバースさせる
			std::reverse(path.begin(), path.end());
			return path;
		}

		openSet.erase(currentIt); // 未探索リストから削除
		closedSet.push_back(current); // 探索済みリストに追加

		// 隣接8方向を探索
		for (int dx = -1; dx <= 1; dx++)
		{
			for (int dz = -1; dz <= 1; dz++)
			{
				if (dx == 0 && dz == 0) continue; // 0,0は自分のグリッドのためスキップ

				int checkX = current->x + dx;
				int checkZ = current->z + dz;

				if (!IsWalkable(checkX, checkZ)) continue; // 障害物がある場合はスキップ

				AStarNode* neighbor = &searchNodes[checkX][checkZ];

				if (std::find(closedSet.begin(), closedSet.end(), neighbor) != closedSet.end()) continue; // 既に探索済みリスト（closedSet）にある場合はスキップ

				float moveCost = (dx != 0 && dz != 0) ? 14.0f : 10.0f;
				float newGCost = current->gCost + moveCost;

				// 探索内容をセット
				// 初期値はありえない値になっている
				if (newGCost < neighbor->gCost)
				{
					neighbor->gCost = newGCost;
					neighbor->hCost = CalculateHeuristic(checkX, checkZ, targetX, targetZ);
					neighbor->parentX = current->x;
					neighbor->parentZ = current->z;

					if (std::find(openSet.begin(), openSet.end(), neighbor) == openSet.end())
					{
						openSet.push_back(neighbor);
					}
				}
			}
		}
	}

	return path;
}

void NavGrid::DrawDebugGrid(const std::vector<VECTOR>& currentPath) const
{
	// グリッド格子の描画
	for (int x = 0; x < width_; ++x)
	{
		for (int z = 0; z < height_; ++z)
		{
			VECTOR pos = GridToWorld(x, z);
			unsigned int color = nodes_[x][z].isWalkable ? GetColor(100, 100, 100) : GetColor(255, 50, 50);

			// 障害物は赤い立方体、通路は小さな枠線で表示
			if (!nodes_[x][z].isWalkable)
			{
				DrawCube3D(
					VGet(pos.x - cellSize_ * 0.45f, 0, pos.z - cellSize_ * 0.45f),
					VGet(pos.x + cellSize_ * 0.45f, 30.0f, pos.z + cellSize_ * 0.45f),
					color, color, TRUE
				);
			}
		}
	}

	// 計算された経路（赤～黄色の太線）の描画
	if (currentPath.size() >= 2)
	{
		for (size_t i = 0; i < currentPath.size() - 1; ++i)
		{
			VECTOR p1 = currentPath[i];
			VECTOR p2 = currentPath[i + 1];
			p1.y += 5.0f; // 地面に埋まらないよう少し浮かせる
			p2.y += 5.0f;
			DrawLine3D(p1, p2, GetColor(255, 255, 0));
			DrawSphere3D(p1, 6.0f, 8, GetColor(255, 100, 0), GetColor(255, 100, 0), TRUE);
		}
	}
}