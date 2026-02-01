#ifndef HIERARCHY_SCOPE_TRACKER_H
#define HIERARCHY_SCOPE_TRACKER_H

#include <vector>
#include <memory>
#include <stdexcept>
#include <cstdio>

/**
 * @brief Tracks hierarchical scope structure using tree.
 *
 * Internal structure has a hidden virtual root ($root).
 * Nodes under virtual root can be system nodes (e.g., $pkg) or user nodes (e.g., tb).
 * All find operations start from user root, not virtual root.
 *
 * @note Currently supports single user root. Multi-root support planned for future.
 */
class HierarchyScopeTracker {
public:
    HierarchyScopeTracker();
    ~HierarchyScopeTracker();

    /**
     * @brief Enter a new scope.
     * @param scopeId Unique identifier for this scope.
     * @param isSystem True for system nodes (e.g., $pkg), false for user nodes.
     */
    void open(int scopeId, bool isSystem = false);

    /**
     * @brief Exit current scope.
     * @param scopeId Expected scope ID for validation.
     * @throws std::runtime_error if scopeId mismatches.
     */
    void close(int scopeId);

    /**
     * @brief Find scope by navigating from user root using indices.
     * @param levelInfo Index path: levelInfo[i] = child index at level i+1.
     * @return scopeId of the target node.
     * @throws std::out_of_range if index is invalid.
     * @note Starts from user root, not virtual root.
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
     * @brief Find user root scope ID.
     * @return scopeId of user root node.
     * @throws std::runtime_error if no user root exists.
     * @note Returns first user node under virtual root.
     * @note TODO: Support multiple user roots in future.
     */
    int findRoot() const;

    /**
     * @brief Get maximum depth of user hierarchy.
     * @return Number of levels (0 if empty), excluding virtual root.
     */
    int getDepth() const;

    /**
     * @brief Check if hierarchy has user nodes.
     * @return true if no user scopes opened.
     */
    bool isEmpty() const;

    /**
     * @brief Print user hierarchy tree structure for debugging.
     * @param fp Output file pointer (e.g., stdout, stderr, or file).
     * @note Only prints user nodes, system nodes are hidden.
     */
    void debugPrint(std::FILE* fp) const;

private:
    /** @brief Tree node structure. */
    struct Node {
        int scopeId;
        bool isSystem;  ///< True for system nodes ($pkg, etc.)
        std::vector<std::unique_ptr<Node>> children;
        Node* parent;

        explicit Node(int id, Node* parentNode = nullptr, bool system = false)
            : scopeId(id), isSystem(system), parent(parentNode) {}
    };

    /** @brief Find node by scopeId (DFS, searches all nodes). */
    const Node* findNode(int scopeId) const;

    /**
     * @brief Find first user root node.
     * @return Pointer to first user node under virtual root, or nullptr.
     * @note TODO: Support multiple user roots in future.
     */
    const Node* findUserRoot() const;

    /**
     * @brief Virtual root node (hidden $root).
     * Always exists, holds both system and user nodes.
     */
    std::unique_ptr<Node> virtualRoot_;

    Node* current_;  ///< Current active node (nullptr means at virtual root level).
};

#endif // HIERARCHY_SCOPE_TRACKER_H
