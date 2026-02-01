#ifndef HIERARCHY_SCOPE_TRACKER_H
#define HIERARCHY_SCOPE_TRACKER_H

#include <vector>
#include <memory>
#include <stdexcept>
#include <cstdio>

/**
 * @brief Tracks hierarchical scope structure using tree.
 */
class HierarchyScopeTracker {
public:
    HierarchyScopeTracker();
    ~HierarchyScopeTracker();

    /**
     * @brief Enter a new scope.
     * @param scopeId Unique identifier for this scope.
     */
    void open(int scopeId);

    /**
     * @brief Exit current scope.
     * @param scopeId Expected scope ID for validation.
     * @throws std::runtime_error if scopeId mismatches.
     */
    void close(int scopeId);

    /**
     * @brief Find scope by navigating from root using indices.
     * @param levelInfo Index path: levelInfo[i] = child index at level i+1.
     * @return scopeId of the target node.
     * @throws std::out_of_range if index is invalid.
     */
    int find(const std::vector<int>& levelInfo) const;

    /**
     * @brief Find scope by navigating from a given scope using indices.
     * @param startScopeId Starting scope to navigate from.
     * @param levelInfo Index path: levelInfo[i] = child index at level i+1.
     * @return scopeId of the target node.
     * @throws std::runtime_error if startScopeId not found.
     * @throws std::out_of_range if index is invalid.
     */
    int find(int startScopeId, const std::vector<int>& levelInfo) const;

    /**
     * @brief Find root scope ID.
     * @return scopeId of root node.
     * @throws std::runtime_error if hierarchy is empty.
     */
    int findRoot() const;

    /**
     * @brief Get maximum depth of hierarchy.
     * @return Number of levels (0 if empty).
     */
    int getDepth() const;

    /**
     * @brief Check if hierarchy is empty.
     * @return true if no scopes opened.
     */
    bool isEmpty() const;

    /**
     * @brief Print hierarchy tree structure for debugging.
     * @param fp Output file pointer (e.g., stdout, stderr, or file).
     */
    void debugPrint(std::FILE* fp) const;

private:
    /** @brief Tree node structure. */
    struct Node {
        int scopeId;
        std::vector<std::unique_ptr<Node>> children;
        Node* parent;

        explicit Node(int id, Node* parentNode = nullptr)
            : scopeId(id), parent(parentNode) {}
    };

    /** @brief Find node by scopeId (DFS). */
    const Node* findNode(int scopeId) const;

    std::unique_ptr<Node> root_;  ///< Root node of hierarchy.
    Node* current_;               ///< Current active node.
};

#endif // HIERARCHY_SCOPE_TRACKER_H
