#ifndef HIERARCHY_SCOPE_TRACKER_H
#define HIERARCHY_SCOPE_TRACKER_H

#include <vector>
#include <memory>
#include <stdexcept>

class HierarchyScopeTracker {
public:
    HierarchyScopeTracker();
    ~HierarchyScopeTracker();

    // Enter a new scope with the given scopeId
    void open(int scopeId);

    // Exit the current scope, scopeId is for validation
    void close(int scopeId);

    // Find a scope by traversing down from root using level indices
    // levelInfo[0] = index of child at level 1
    // levelInfo[1] = index of child at level 2, etc.
    // Returns the scopeId of the found node
    int find(const std::vector<int>& levelInfo) const;

    // Get the depth of the hierarchy (number of levels)
    int getDepth() const;

    // Check if the tracker is empty (no scopes opened)
    bool isEmpty() const;

private:
    struct Node {
        int scopeId;
        std::vector<std::unique_ptr<Node>> children;
        Node* parent;

        explicit Node(int id, Node* parentNode = nullptr)
            : scopeId(id), parent(parentNode) {}
    };

    std::unique_ptr<Node> root_;
    Node* current_;
};

#endif // HIERARCHY_SCOPE_TRACKER_H
