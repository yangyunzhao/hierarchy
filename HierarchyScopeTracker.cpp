#include "HierarchyScopeTracker.h"
#include <functional>

HierarchyScopeTracker::HierarchyScopeTracker()
    : root_(nullptr), current_(nullptr) {
}

HierarchyScopeTracker::~HierarchyScopeTracker() = default;

void HierarchyScopeTracker::open(int scopeId) {
    auto newNode = std::make_unique<Node>(scopeId, current_);
    Node* newNodePtr = newNode.get();

    if (current_ == nullptr) {
        // First node, add to root level
        if (root_ == nullptr) {
            root_ = std::move(newNode);
        } else {
            // We need a virtual root to hold multiple top-level nodes
            // For simplicity, we'll treat the first opened scope as root
            // and subsequent opens at root level as siblings
            // Actually, let's use a different approach: root_ is a virtual root
            // that holds all top-level scopes
            throw std::runtime_error("Cannot open multiple root scopes without closing");
        }
        current_ = newNodePtr;
    } else {
        current_->children.push_back(std::move(newNode));
        current_ = newNodePtr;
    }
}

void HierarchyScopeTracker::close(int scopeId) {
    if (current_ == nullptr) {
        throw std::runtime_error("No scope to close");
    }

    if (current_->scopeId != scopeId) {
        throw std::runtime_error("Scope ID mismatch: expected " +
            std::to_string(current_->scopeId) + ", got " + std::to_string(scopeId));
    }

    current_ = current_->parent;
}

int HierarchyScopeTracker::find(const std::vector<int>& levelInfo) const {
    if (root_ == nullptr) {
        throw std::runtime_error("Hierarchy is empty");
    }

    if (levelInfo.empty()) {
        throw std::runtime_error("Level info cannot be empty");
    }

    // Start from root
    const Node* node = root_.get();

    // The first index in levelInfo selects from root's children
    // But wait, if levelInfo = {1, 5, 3}, it means:
    // - Level 1: select child at index 1
    // - Level 2: select child at index 5
    // - Level 3: select child at index 3

    // Actually, based on the description:
    // "当前层的下一层(有多个子节点,选第1个节点(从0开始))"
    // This means levelInfo[0] = 1 means select the 2nd child (index 1) of the current level

    // If we start from root (which is level 0), then levelInfo[0] selects among root's children
    // But root itself is a single node, so levelInfo[0] should select among root's children

    // Let me reconsider: the root is level 0
    // levelInfo = {1, 5, 3} means:
    // - From root (level 0), go to child at index 1 (this is level 1)
    // - From that node, go to child at index 5 (this is level 2)
    // - From that node, go to child at index 3 (this is level 3)
    // Return the scopeId of the final node

    // Wait, but in the current design, root_ IS the first opened scope, not a virtual root.
    // So if user does:
    // open(100) -> root is scope 100
    // open(200) -> child of 100
    // open(300) -> child of 200
    // close(300)
    // open(301) -> another child of 200
    // close(301)
    // close(200)
    // open(201) -> another child of 100
    // ...

    // If find({0}) is called, it should return scope 200 (first child of root 100)
    // If find({0, 0}) is called, it should return scope 300 (first child of 200)
    // If find({0, 1}) is called, it should return scope 301 (second child of 200)
    // If find({1}) is called, it should return scope 201 (second child of root 100)

    // So the current node starts as root_, and we navigate down using indices

    for (size_t i = 0; i < levelInfo.size(); ++i) {
        int index = levelInfo[i];

        if (index < 0 || static_cast<size_t>(index) >= node->children.size()) {
            throw std::out_of_range("Index " + std::to_string(index) +
                " out of range at level " + std::to_string(i + 1) +
                " (available: " + std::to_string(node->children.size()) + " children)");
        }

        node = node->children[index].get();
    }

    return node->scopeId;
}

int HierarchyScopeTracker::getDepth() const {
    if (root_ == nullptr) {
        return 0;
    }

    int maxDepth = 1;

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
