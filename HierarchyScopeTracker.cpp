#include "HierarchyScopeTracker.h"
#include <functional>

HierarchyScopeTracker::HierarchyScopeTracker()
    : root_(nullptr), current_(nullptr) {
}

HierarchyScopeTracker::~HierarchyScopeTracker() = default;

void HierarchyScopeTracker::open(int scopeId) {
    // 创建新节点，父节点为当前节点
    auto newNode = std::make_unique<Node>(scopeId, current_);
    Node* newNodePtr = newNode.get();

    if (current_ == nullptr) {
        // 当前无活动节点，设置为根节点
        if (root_ == nullptr) {
            root_ = std::move(newNode);
        } else {
            // 不允许多个根节点
            throw std::runtime_error("Cannot open multiple root scopes without closing");
        }
        current_ = newNodePtr;
    } else {
        // 添加为当前节点的子节点
        current_->children.push_back(std::move(newNode));
        current_ = newNodePtr;
    }
}

void HierarchyScopeTracker::close(int scopeId) {
    // 检查是否有可关闭的scope
    if (current_ == nullptr) {
        throw std::runtime_error("No scope to close");
    }

    // 校验scopeId是否匹配
    if (current_->scopeId != scopeId) {
        throw std::runtime_error("Scope ID mismatch: expected " +
            std::to_string(current_->scopeId) + ", got " + std::to_string(scopeId));
    }

    // 返回父节点
    current_ = current_->parent;
}

int HierarchyScopeTracker::find(const std::vector<int>& levelInfo) const {
    if (root_ == nullptr) {
        throw std::runtime_error("Hierarchy is empty");
    }

    if (levelInfo.empty()) {
        throw std::runtime_error("Level info cannot be empty");
    }

    // 从根节点开始遍历
    const Node* node = root_.get();

    // 按levelInfo逐层向下查找
    // levelInfo[i] 表示在第i+1层选择第几个子节点(从0开始)
    for (size_t i = 0; i < levelInfo.size(); ++i) {
        int index = levelInfo[i];

        // 检查索引是否有效
        if (index < 0 || static_cast<size_t>(index) >= node->children.size()) {
            throw std::out_of_range("Index " + std::to_string(index) +
                " out of range at level " + std::to_string(i + 1) +
                " (available: " + std::to_string(node->children.size()) + " children)");
        }

        node = node->children[index].get();
    }

    return node->scopeId;
}

int HierarchyScopeTracker::findRoot() const {
    // 检查层级结构是否为空
    if (root_ == nullptr) {
        throw std::runtime_error("Hierarchy is empty");
    }

    // 返回根节点的scopeId
    return root_->scopeId;
}

int HierarchyScopeTracker::getDepth() const {
    if (root_ == nullptr) {
        return 0;
    }

    int maxDepth = 1;

    // 递归遍历所有节点，找出最大深度
    std::function<void(const Node*, int)> traverse = [&](const Node* node, int depth) {
        if (depth > maxDepth) {
            maxDepth = depth;
        }
        for (const auto& child : node->children) {
            traverse(child.get(), depth + 1);
        }
    };

    traverse(root_.get(), 1);
    return maxDepth;
}

bool HierarchyScopeTracker::isEmpty() const {
    return root_ == nullptr;
}
