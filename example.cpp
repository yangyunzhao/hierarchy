#include "HierarchyScopeTracker.h"
#include <iostream>
#include <iomanip>

/*
 * 示例程序：演示HierarchyScopeTracker的使用
 *
 * 构建的树结构如下 (scopeId标注在节点上):
 *
 * 1 (root)
 * ├── 10
 * │   ├── 100
 * │   │   └── 1000
 * │   ├── 101
 * │   └── 102
 * ├── 11
 * │   ├── 110
 * │   ├── 111
 * │   ├── 112
 * │   ├── 113
 * │   ├── 114
 * │   └── 115
 * │       ├── 1150
 * │       ├── 1151
 * │       ├── 1152
 * │       └── 1153
 * └── 12
 *
 * 索引说明 (从0开始):
 * - find({0})       -> 10   (root的第0个子节点)
 * - find({1})       -> 11   (root的第1个子节点)
 * - find({2})       -> 12   (root的第2个子节点)
 * - find({0, 0})    -> 100  (10的第0个子节点)
 * - find({0, 0, 0}) -> 1000 (100的第0个子节点)
 * - find({1, 5})    -> 115  (11的第5个子节点)
 * - find({1, 5, 3}) -> 1153 (115的第3个子节点)
 */

void buildTree(HierarchyScopeTracker& tracker) {
    tracker.open(1);      // root
        tracker.open(10);     // child 0 of root
            tracker.open(100);    // child 0 of 10
                tracker.open(1000);   // child 0 of 100
                tracker.close(1000);
            tracker.close(100);
            tracker.open(101);    // child 1 of 10
            tracker.close(101);
            tracker.open(102);    // child 2 of 10
            tracker.close(102);
        tracker.close(10);
        tracker.open(11);     // child 1 of root
            for (int i = 0; i < 6; ++i) {
                tracker.open(110 + i);  // children 110-115
                if (i == 5) {
                    // 在115下创建子节点
                    for (int j = 0; j < 4; ++j) {
                        tracker.open(1150 + j);
                        tracker.close(1150 + j);
                    }
                }
                tracker.close(110 + i);
            }
        tracker.close(11);
        tracker.open(12);     // child 2 of root
        tracker.close(12);
    tracker.close(1);
}

void testFind(const HierarchyScopeTracker& tracker,
              const std::vector<int>& levelInfo,
              int expected) {
    // 打印查询路径
    std::cout << "find({";
    for (size_t i = 0; i < levelInfo.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << levelInfo[i];
    }
    std::cout << "})";

    int result = tracker.find(levelInfo);
    std::cout << " = " << std::setw(4) << result;

    // 验证结果
    if (result == expected) {
        std::cout << "  [OK]" << std::endl;
    } else {
        std::cout << "  [FAIL] expected " << expected << std::endl;
    }
}

int main() {
    std::cout << "=== HierarchyScopeTracker Example ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Tree structure (scopeId):" << std::endl;
    std::cout << std::endl;
    std::cout << "1 (root)" << std::endl;
    std::cout << "├── 10" << std::endl;
    std::cout << "│   ├── 100" << std::endl;
    std::cout << "│   │   └── 1000" << std::endl;
    std::cout << "│   ├── 101" << std::endl;
    std::cout << "│   └── 102" << std::endl;
    std::cout << "├── 11" << std::endl;
    std::cout << "│   ├── 110" << std::endl;
    std::cout << "│   ├── 111" << std::endl;
    std::cout << "│   ├── 112" << std::endl;
    std::cout << "│   ├── 113" << std::endl;
    std::cout << "│   ├── 114" << std::endl;
    std::cout << "│   └── 115" << std::endl;
    std::cout << "│       ├── 1150" << std::endl;
    std::cout << "│       ├── 1151" << std::endl;
    std::cout << "│       ├── 1152" << std::endl;
    std::cout << "│       └── 1153" << std::endl;
    std::cout << "└── 12" << std::endl;
    std::cout << std::endl;

    // 构建树
    HierarchyScopeTracker tracker;
    buildTree(tracker);

    // 测试findRoot
    std::cout << "--- findRoot ---" << std::endl;
    std::cout << "findRoot() = " << tracker.findRoot() << "  [expected: 1]" << std::endl;
    std::cout << std::endl;

    // 测试getDepth
    std::cout << "--- getDepth ---" << std::endl;
    std::cout << "getDepth() = " << tracker.getDepth() << "  [expected: 4]" << std::endl;
    std::cout << std::endl;

    // 测试find
    std::cout << "--- find queries ---" << std::endl;
    testFind(tracker, {0},          10);
    testFind(tracker, {1},          11);
    testFind(tracker, {2},          12);
    testFind(tracker, {0, 0},       100);
    testFind(tracker, {0, 1},       101);
    testFind(tracker, {0, 2},       102);
    testFind(tracker, {0, 0, 0},    1000);
    testFind(tracker, {1, 0},       110);
    testFind(tracker, {1, 5},       115);
    testFind(tracker, {1, 5, 0},    1150);
    testFind(tracker, {1, 5, 3},    1153);  // 用户示例中的查询

    std::cout << std::endl;
    std::cout << "=== Example Complete ===" << std::endl;

    return 0;
}
