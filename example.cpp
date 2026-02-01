#include "HierarchyScopeTracker.h"
#include <iostream>
#include <iomanip>

/*
 * 示例程序：演示HierarchyScopeTracker的使用
 *
 * 内部结构 (包含隐藏的虚拟根和系统节点):
 *
 * $root (虚拟根，隐藏)
 * ├── $pkg (系统节点, scopeId=9000, isSystem=true)
 * ├── $unit (系统节点, scopeId=9001, isSystem=true)
 * └── tb (用户节点, scopeId=1) <-- 用户根，find/findRoot从这里开始
 *     ├── top (scopeId=10)
 *     │   ├── dut (scopeId=100)
 *     │   │   └── core (scopeId=1000)
 *     │   ├── mem (scopeId=101)
 *     │   └── io (scopeId=102)
 *     ├── monitor (scopeId=11)
 *     │   ├── checker0 (scopeId=110)
 *     │   ├── checker1 (scopeId=111)
 *     │   ├── checker2 (scopeId=112)
 *     │   ├── checker3 (scopeId=113)
 *     │   ├── checker4 (scopeId=114)
 *     │   └── checker5 (scopeId=115)
 *     │       ├── sub0 (scopeId=1150)
 *     │       ├── sub1 (scopeId=1151)
 *     │       ├── sub2 (scopeId=1152)
 *     │       └── sub3 (scopeId=1153)
 *     └── driver (scopeId=12)
 *
 * debugPrint只显示用户节点:
 * 1
 * ├── 10
 * │   ├── 100
 * │   │   └── 1000
 * │   ├── 101
 * │   └── 102
 * ├── 11
 * │   ├── 110
 * │   ...
 * │   └── 115
 * │       └── 1153
 * └── 12
 */

void buildTree(HierarchyScopeTracker& tracker) {
    // 系统节点 (isSystem=true)
    tracker.open(9000, true);   // $pkg
    tracker.close(9000);

    tracker.open(9001, true);   // $unit
    tracker.close(9001);

    // 用户节点 (isSystem=false, 默认值)
    tracker.open(1);            // tb (用户根)
        tracker.open(10);       // top
            tracker.open(100);  // dut
                tracker.open(1000);  // core
                tracker.close(1000);
            tracker.close(100);
            tracker.open(101);  // mem
            tracker.close(101);
            tracker.open(102);  // io
            tracker.close(102);
        tracker.close(10);
        tracker.open(11);       // monitor
            for (int i = 0; i < 6; ++i) {
                tracker.open(110 + i);  // checker0-5
                if (i == 5) {
                    for (int j = 0; j < 4; ++j) {
                        tracker.open(1150 + j);  // sub0-3
                        tracker.close(1150 + j);
                    }
                }
                tracker.close(110 + i);
            }
        tracker.close(11);
        tracker.open(12);       // driver
        tracker.close(12);
    tracker.close(1);
}

void testFind(const HierarchyScopeTracker& tracker,
              const std::vector<int>& levelInfo,
              int expected) {
    std::cout << "find({";
    for (size_t i = 0; i < levelInfo.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << levelInfo[i];
    }
    std::cout << "})";

    int result = tracker.find(levelInfo);
    std::cout << " = " << std::setw(4) << result;

    if (result == expected) {
        std::cout << "  [OK]" << std::endl;
    } else {
        std::cout << "  [FAIL] expected " << expected << std::endl;
    }
}

void testFindFrom(const HierarchyScopeTracker& tracker,
                  int startScopeId,
                  const std::vector<int>& levelInfo,
                  int expected) {
    std::cout << "find(" << startScopeId << ", {";
    for (size_t i = 0; i < levelInfo.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << levelInfo[i];
    }
    std::cout << "})";

    int result = tracker.find(startScopeId, levelInfo);
    std::cout << " = " << std::setw(4) << result;

    if (result == expected) {
        std::cout << "  [OK]" << std::endl;
    } else {
        std::cout << "  [FAIL] expected " << expected << std::endl;
    }
}

int main() {
    std::cout << "=== HierarchyScopeTracker Example ===" << std::endl;
    std::cout << std::endl;

    // 构建树 (包含系统节点和用户节点)
    HierarchyScopeTracker tracker;
    buildTree(tracker);

    // debugPrint只显示用户节点
    std::cout << "--- debugPrint(stdout) - 只显示用户节点 ---" << std::endl;
    tracker.debugPrint(stdout);
    std::cout << std::endl;

    // findRoot返回用户根 (tb)，不是系统节点 ($pkg)
    std::cout << "--- findRoot - 返回用户根 ---" << std::endl;
    std::cout << "findRoot() = " << tracker.findRoot() << "  [expected: 1 (tb)]" << std::endl;
    std::cout << std::endl;

    // getDepth从用户根开始计算
    std::cout << "--- getDepth - 从用户根计算 ---" << std::endl;
    std::cout << "getDepth() = " << tracker.getDepth() << "  [expected: 4]" << std::endl;
    std::cout << std::endl;

    // find从用户根开始查找
    std::cout << "--- find(levelInfo) - 从用户根开始 ---" << std::endl;
    testFind(tracker, {0},          10);     // top
    testFind(tracker, {1},          11);     // monitor
    testFind(tracker, {2},          12);     // driver
    testFind(tracker, {0, 0},       100);    // dut
    testFind(tracker, {0, 1},       101);    // mem
    testFind(tracker, {0, 2},       102);    // io
    testFind(tracker, {0, 0, 0},    1000);   // core
    testFind(tracker, {1, 0},       110);    // checker0
    testFind(tracker, {1, 5},       115);    // checker5
    testFind(tracker, {1, 5, 0},    1150);   // sub0
    testFind(tracker, {1, 5, 3},    1153);   // sub3
    std::cout << std::endl;

    // find(scopeId, levelInfo)可以从任意节点开始，包括系统节点
    std::cout << "--- find(scopeId, levelInfo) - 从任意节点 ---" << std::endl;
    std::cout << std::endl;

    std::cout << "从monitor(11)出发:" << std::endl;
    testFindFrom(tracker, 11, {0},       110);   // checker0
    testFindFrom(tracker, 11, {5},       115);   // checker5
    testFindFrom(tracker, 11, {5, 3},    1153);  // sub3
    std::cout << std::endl;

    std::cout << "从top(10)出发:" << std::endl;
    testFindFrom(tracker, 10, {0},       100);   // dut
    testFindFrom(tracker, 10, {0, 0},    1000);  // core
    testFindFrom(tracker, 10, {1},       101);   // mem
    std::cout << std::endl;

    std::cout << "空levelInfo返回起始节点本身:" << std::endl;
    testFindFrom(tracker, 11, {},        11);
    testFindFrom(tracker, 1153, {},      1153);
    std::cout << std::endl;

    std::cout << "find({})返回用户根:" << std::endl;
    testFind(tracker, {},           1);      // tb (user root)

    std::cout << std::endl;
    std::cout << "=== Example Complete ===" << std::endl;

    return 0;
}
