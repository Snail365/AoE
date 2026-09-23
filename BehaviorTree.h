#pragma once
#include <vector>
#include <memory>
#include <functional>

// ノードの実行状態
enum class NodeStatus
{
    Success,
    Failure,
    Running
};

// ノードの基底クラス
class BTNode
{
public:
    virtual ~BTNode() = default;
    virtual NodeStatus Tick() = 0;
};

// ------------------------------------------------------------
// 制御ノード: Composite（複合ノード）の基底クラス
// ------------------------------------------------------------
class BTCompositeNode : public BTNode
{
public:
    void AddChild(std::shared_ptr<BTNode> child) { children_.push_back(child); }

protected:
    std::vector<std::shared_ptr<BTNode>> children_;
};

// 【Selector ノード】
// 子ノードを順番に実行し、成功(SUCCESS)または実行中(RUNNING)を返した時点で評価を中断してそれを返す。
// 全て失敗(FAILURE)したら FAILURE を返す。（優先度付き選択）
class BTSelector : public BTCompositeNode
{
public:
    NodeStatus Tick() override
    {
        for (auto& child : children_)
        {
            NodeStatus status = child->Tick();
            if (status != NodeStatus::Failure)
            {
                return status;
            }
        }
        return NodeStatus::Failure;
    }
};

// 【Sequence ノード】
// 子ノードを順番に実行し、失敗(FAILURE)または実行中(RUNNING)を返した時点で評価を中断してそれを返す。
// 全て成功(SUCCESS)したら SUCCESS を返す。（一連のシーケンス動作）
class BTSequence : public BTCompositeNode
{
public:
    NodeStatus Tick() override
    {
        for (auto& child : children_)
        {
            NodeStatus status = child->Tick();
            if (status != NodeStatus::Success)
            {
                return status;
            }
        }
        return NodeStatus::Success;
    }
};

// ------------------------------------------------------------
// 葉ノード: Action (行動) / Condition (条件判定)
// ------------------------------------------------------------

// 【Action ノード】（ラムダ式等で柔軟に行動を定義できる設計）
class BTAction : public BTNode
{
public:
    using ActionFunc = std::function<NodeStatus()>;
    explicit BTAction(ActionFunc action) : action_(action) {}

    NodeStatus Tick() override
    {
        return action_ ? action_() : NodeStatus::Failure;
    }

private:
    ActionFunc action_;
};

// 【Condition ノード】（ラムダ式等で柔軟に条件を定義できる設計）
class BTCondition : public BTNode
{
public:
    using ConditionFunc = std::function<bool()>;
    explicit BTCondition(ConditionFunc condition) : condition_(condition) {}

    NodeStatus Tick() override
    {
        if (condition_ && condition_())
        {
            return NodeStatus::Success;
        }
        return NodeStatus::Failure;
    }

private:
    ConditionFunc condition_;
};