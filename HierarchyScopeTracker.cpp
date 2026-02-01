#include "HierarchyScopeTracker.h"
#include <functional>
#include <string>

// 虚拟根节点的特殊scopeId（内部使用，不对外暴露）
static const int VIRTUAL_ROOT_ID = -1;

HierarchyScopeTracker::HierarchyScopeTracker()
    : virtualRoot_(std::make_unique<Node>(VIRTUAL_ROOT_ID, nullptr, true)),
      current_(nullptr) {
    // 虚拟根节点始终存在，标记为系统节点
}

HierarchyScopeTracker::~HierarchyScopeTracker() = default;

void HierarchyScopeTracker::open(int scopeId, bool isSystem) {
    // 创建新节点
    Node* parentNode = (current_ == nullptr) ? virtualRoot_.get() : current_;
    auto newNode = std::make_unique<Node>(scopeId, parentNode, isSystem);
    Node* newNodePtr = newNode.get();

    // 添加到父节点的子节点列表
    parentNode->children.push_back(std::move(newNode));
    current_ = newNodePtr;
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

    // 返回父节点（如果父节点是虚拟根，则设为nullptr）
    if (current_->parent == virtualRoot_.get()) {
        current_ = nullptr;
    } else {
        current_ = current_->parent;
    }
}

const HierarchyScopeTracker::Node* HierarchyScopeTracker::findUserRoot() const {
    // 在虚拟根的子节点中查找第一个用户节点
    // TODO: 未来支持多用户根时，可能需要返回列表或提供索引参数
    for (const auto& child : virtualRoot_->children) {
        if (!child->isSystem) {
            return child.get();
        }
    }
    return nullptr;
}

int HierarchyScopeTracker::find(const std::vector<int>& levelInfo) const {
    // 从用户根开始查找
    const Node* userRoot = findUserRoot();
    if (userRoot == nullptr) {
        throw std::runtime_error("No user root exists");
    }

    // 如果levelInfo为空，返回用户根
    if (levelInfo.empty()) {
        return userRoot->scopeId;
    }

    // 从用户根开始，按levelInfo逐层向下查找
    const Node* node = userRoot;

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

const HierarchyScopeTracker::Node* HierarchyScopeTracker::findNode(int scopeId) const {
    // 深度优先搜索查找指定scopeId的节点（搜索所有节点，包括系统节点）
    std::function<const Node*(const Node*)> search = [&](const Node* node) -> const Node* {
        if (node->scopeId == scopeId) {
            return node;
        }
        for (const auto& child : node->children) {
            const Node* found = search(child.get());
            if (found != nullptr) {
                return found;
            }
        }
        return nullptr;
    };

    // 从虚拟根的子节点开始搜索
    for (const auto& child : virtualRoot_->children) {
        const Node* found = search(child.get());
        if (found != nullptr) {
            return found;
        }
    }
    return nullptr;
}

int HierarchyScopeTracker::find(int startScopeId, const std::vector<int>& levelInfo) const {
    // 查找起始节点
    const Node* node = findNode(startScopeId);
    if (node == nullptr) {
        throw std::runtime_error("Scope ID " + std::to_string(startScopeId) + " not found");
    }

    // 如果levelInfo为空，直接返回起始节点的scopeId
    if (levelInfo.empty()) {
        return node->scopeId;
    }

    // 从起始节点按levelInfo逐层向下查找
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
    // 返回用户根节点的scopeId
    const Node* userRoot = findUserRoot();
    if (userRoot == nullptr) {
        throw std::runtime_error("No user root exists");
    }
    return userRoot->scopeId;
}

int HierarchyScopeTracker::getDepth() const {
    // 从用户根开始计算深度
    const Node* userRoot = findUserRoot();
    if (userRoot == nullptr) {
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

    traverse(userRoot, 1);
    return maxDepth;
}

bool HierarchyScopeTracker::isEmpty() const {
    // 检查是否存在用户根节点
    return findUserRoot() == nullptr;
}

void HierarchyScopeTracker::debugPrint(std::FILE* fp) const {
    // 只打印用户节点，从用户根开始
    const Node* userRoot = findUserRoot();
    if (userRoot == nullptr) {
        std::fprintf(fp, "(empty)\n");
        return;
    }

    // 递归打印树结构
    // prefix: 当前行前缀, isLast: 是否为父节点的最后一个子节点
    std::function<void(const Node*, const std::string&, bool)> print =
        [&](const Node* node, const std::string& prefix, bool isLast) {
        // 打印当前节点
        std::fprintf(fp, "%s", prefix.c_str());
        std::fprintf(fp, "%s", isLast ? "└── " : "├── ");
        std::fprintf(fp, "%d\n", node->scopeId);

        // 计算子节点的前缀
        std::string childPrefix = prefix + (isLast ? "    " : "│   ");

        // 递归打印子节点
        for (size_t i = 0; i < node->children.size(); ++i) {
            bool childIsLast = (i == node->children.size() - 1);
            print(node->children[i].get(), childPrefix, childIsLast);
        }
    };

    // 打印用户根节点
    std::fprintf(fp, "%d\n", userRoot->scopeId);

    // 打印用户根节点的子节点
    for (size_t i = 0; i < userRoot->children.size(); ++i) {
        bool isLast = (i == userRoot->children.size() - 1);
        print(userRoot->children[i].get(), "", isLast);
    }
}
