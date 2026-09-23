#include "Enemy.h"
#include "GameSystemManager.h"
#include "AoECircle.h"

Enemy::Enemy(FileManager& fileManager, int id, const VECTOR& startPos, int hp, float attackInterval)
    : fileMng_(fileManager)
{
    Initialize(id, startPos, hp, attackInterval);
}

void Enemy::Initialize(int id, const VECTOR& startPos, int hp, float attackInterval)
{
    data_.id = id;
    data_.pos = startPos;
    data_.hp = hp;
    data_.maxHp = hp;
    data_.attackInterval = attackInterval;
    data_.attackTimer = 0.0f;
    data_.speed = 100.0f + (id % 3) * 20.0f; // 速度に少しバラつきを持たせる
    data_.isAlive = true;

    // A*再計算タイマーの初期値をバラけさせて処理負荷を分散
    pathUpdateTimer_ = (id % 5) * 0.05f;

    imageFile_ = fileMng_.LoadImageFM("Resource/Enemy/Enemy.png");

    BuildBehaviorTree();
}

void Enemy::BuildBehaviorTree()
{
    auto rootSelector = std::make_shared<BTSelector>();

    // ------------------------------------------------------------
    // 思考 1: 攻撃（120 unit 以内 & リロード完了）
    // ------------------------------------------------------------
    auto attackSequence = std::make_shared<BTSequence>();

    attackSequence->AddChild(std::make_shared<BTCondition>([this]()
                                                           {
                                                               return data_.attackTimer >= data_.attackInterval;
                                                           }));

    attackSequence->AddChild(std::make_shared<BTCondition>([this]()
                                                           {
                                                               float dist = VSize(VSub(currentTargetPos_, data_.pos));
                                                               return dist <= 120.0f;
                                                           }));

    attackSequence->AddChild(std::make_shared<BTAction>([this]()
                                                        {
                                                            data_.attackTimer = 0.0f;
                                                            currentStateName_ = "Attacking";
                                                            return NodeStatus::Success;
                                                        }));

    // ------------------------------------------------------------
    // 思考 2: A* 追尾
    // ------------------------------------------------------------
    auto chaseAction = std::make_shared<BTAction>([this]()
                                                  {
                                                      if (!currentPath_.empty())
                                                      {
                                                          currentStateName_ = "Chasing (A*)";
                                                          VECTOR nextWayPoint = currentPath_[0];
                                                          VECTOR toPoint = VSub(nextWayPoint, data_.pos);
                                                          toPoint.y = 0;

                                                          if (VSize(toPoint) < 12.0f)
                                                          {
                                                              currentPath_.erase(currentPath_.begin());
                                                          }
                                                          else
                                                          {
                                                              MoveTo(nextWayPoint, currentDeltaTime_);
                                                          }
                                                          return NodeStatus::Running;
                                                      }
                                                      currentStateName_ = "Searching Path";
                                                      return NodeStatus::Failure;
                                                  });

    rootSelector->AddChild(attackSequence);
    rootSelector->AddChild(chaseAction);

    treeRoot_ = rootSelector;
}

void Enemy::Update(float deltaTime, const VECTOR& playerPos, const NavGrid& navGrid, GameSystemManager& systemMng)
{
    if (!data_.isAlive) return;

    currentDeltaTime_ = deltaTime;
    currentTargetPos_ = playerPos;
    data_.attackTimer += deltaTime;

    // A* 経路の定期再計算 (0.2秒ごと)
    pathUpdateTimer_ += deltaTime;
    if (pathUpdateTimer_ >= 0.2f)
    {
        pathUpdateTimer_ = 0.0f;
        currentPath_ = const_cast<NavGrid&>(navGrid).FindPath(data_.pos, playerPos);
        if (!currentPath_.empty()) currentPath_.erase(currentPath_.begin());
    }

    if (treeRoot_)
    {
        NodeStatus status = treeRoot_->Tick();

        if (status == NodeStatus::Success && currentStateName_ == "Attacking")
        {
            systemMng.SpawnAoE(std::make_unique<AoECircle>(playerPos, 100.0f, 2.0f));
        }
    }
}

void Enemy::Draw3D() const
{
    if (!data_.isAlive) return;

    if (imageFile_ != nullptr)
    {
        VECTOR drawPos = VGet(data_.pos.x, data_.pos.y + size_ * 0.5f, data_.pos.z);
        DrawBillboard3D(drawPos, 0.5f, 0.5f, size_, 0.0f, imageFile_->GetHandle(), TRUE);
    }
    else
    {
        DrawSphere3D(VGet(data_.pos.x, data_.pos.y + 15.0f, data_.pos.z), 15.0f, 16, GetColor(255, 80, 80), GetColor(255, 255, 255), TRUE);
    }
}

// ★ 各敵専用のA* 経路 & ターゲットデバッグ描画
void Enemy::DrawDebug3D() const
{
    if (!data_.isAlive) return;

    // 1. 各敵が通ろうとしているA*ルート（緑〜シアンの線）
    if (currentPath_.size() >= 2)
    {
        for (size_t i = 0; i < currentPath_.size() - 1; ++i)
        {
            VECTOR p1 = VGet(currentPath_[i].x, currentPath_[i].y + 2.0f, currentPath_[i].z);
            VECTOR p2 = VGet(currentPath_[i + 1].x, currentPath_[i + 1].y + 2.0f, currentPath_[i + 1].z);
            DrawLine3D(p1, p2, GetColor(50, 255, 150));
            DrawSphere3D(p1, 3.0f, 6, GetColor(0, 255, 200), GetColor(0, 255, 200), TRUE);
        }
    }

    // 2. 現在目指している直近の WayPoint（黄色ライン）
    if (!currentPath_.empty())
    {
        VECTOR targetPt = VGet(currentPath_[0].x, currentPath_[0].y + 5.0f, currentPath_[0].z);
        VECTOR enemyPt = VGet(data_.pos.x, data_.pos.y + 5.0f, data_.pos.z);
        DrawLine3D(enemyPt, targetPt, GetColor(255, 255, 0));
    }
}

void Enemy::MoveTo(const VECTOR& targetPos, float deltaTime)
{
    VECTOR dir = VSub(targetPos, data_.pos);
    dir.y = 0;
    if (VSize(dir) > 0.001f)
    {
        dir = VNorm(dir);
        data_.pos = VAdd(data_.pos, VScale(dir, data_.speed * deltaTime));
    }
}

void Enemy::TakeDamage(int damage)
{
    if (!data_.isAlive) return;
    data_.hp = (data_.hp > damage) ? data_.hp - damage : 0;
    if (data_.hp == 0) data_.isAlive = false;
}