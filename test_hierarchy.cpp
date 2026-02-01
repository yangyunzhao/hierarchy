#define BOOST_TEST_MODULE HierarchyScopeTrackerTest
#include <boost/test/included/unit_test.hpp>
#include "HierarchyScopeTracker.h"

BOOST_AUTO_TEST_SUITE(HierarchyScopeTrackerTests)

// Test basic open and close operations
BOOST_AUTO_TEST_CASE(TestBasicOpenClose) {
    HierarchyScopeTracker tracker;

    BOOST_CHECK(tracker.isEmpty());

    tracker.open(100);
    BOOST_CHECK(!tracker.isEmpty());

    tracker.close(100);
    // After closing root, current_ is nullptr but root_ still exists
    BOOST_CHECK(!tracker.isEmpty());
}

// Test scope ID mismatch on close
BOOST_AUTO_TEST_CASE(TestCloseMismatch) {
    HierarchyScopeTracker tracker;

    tracker.open(100);

    BOOST_CHECK_THROW(tracker.close(999), std::runtime_error);

    // Correct close
    tracker.close(100);
}

// Test closing when no scope is open
BOOST_AUTO_TEST_CASE(TestCloseEmpty) {
    HierarchyScopeTracker tracker;

    BOOST_CHECK_THROW(tracker.close(100), std::runtime_error);
}

// Test nested scopes
BOOST_AUTO_TEST_CASE(TestNestedScopes) {
    HierarchyScopeTracker tracker;

    tracker.open(100);  // Level 0 (root)
    tracker.open(200);  // Level 1
    tracker.open(300);  // Level 2

    // Close in reverse order
    tracker.close(300);
    tracker.close(200);
    tracker.close(100);

    BOOST_CHECK(!tracker.isEmpty());
}

// Test find with single level
BOOST_AUTO_TEST_CASE(TestFindSingleLevel) {
    HierarchyScopeTracker tracker;

    tracker.open(100);   // root
    tracker.open(200);   // child 0 of root
    tracker.close(200);
    tracker.open(201);   // child 1 of root
    tracker.close(201);
    tracker.open(202);   // child 2 of root
    tracker.close(202);
    tracker.close(100);

    BOOST_CHECK_EQUAL(tracker.find({0}), 200);
    BOOST_CHECK_EQUAL(tracker.find({1}), 201);
    BOOST_CHECK_EQUAL(tracker.find({2}), 202);
}

// Test find with multiple levels - example from requirement: find({1, 5, 3})
BOOST_AUTO_TEST_CASE(TestFindMultipleLevels) {
    HierarchyScopeTracker tracker;

    tracker.open(1);  // root

    // Create children at first level (indices 0, 1)
    tracker.open(10);  // child 0
    tracker.close(10);

    tracker.open(11);  // child 1 - this is where we go with levelInfo[0]=1

    // Create children at second level (indices 0..5)
    for (int i = 0; i < 6; ++i) {
        tracker.open(110 + i);  // children 110, 111, 112, 113, 114, 115
        if (i == 5) {
            // At index 5 (scope 115), create children at third level
            for (int j = 0; j < 4; ++j) {
                tracker.open(1150 + j);  // children 1150, 1151, 1152, 1153
                tracker.close(1150 + j);
            }
        }
        tracker.close(110 + i);
    }

    tracker.close(11);
    tracker.close(1);

    // find({1, 5, 3}) should return 1153
    // - Start at root (1)
    // - Go to child index 1 -> scope 11
    // - Go to child index 5 -> scope 115
    // - Go to child index 3 -> scope 1153
    BOOST_CHECK_EQUAL(tracker.find({1, 5, 3}), 1153);

    // Also test intermediate finds
    BOOST_CHECK_EQUAL(tracker.find({1}), 11);
    BOOST_CHECK_EQUAL(tracker.find({1, 5}), 115);
    BOOST_CHECK_EQUAL(tracker.find({1, 0}), 110);
}

// Test find with out of range index
BOOST_AUTO_TEST_CASE(TestFindOutOfRange) {
    HierarchyScopeTracker tracker;

    tracker.open(100);
    tracker.open(200);
    tracker.close(200);
    tracker.close(100);

    // Only one child at index 0
    BOOST_CHECK_THROW(tracker.find({1}), std::out_of_range);
    BOOST_CHECK_THROW(tracker.find({0, 0}), std::out_of_range);  // 200 has no children
}

// Test find on empty tracker
BOOST_AUTO_TEST_CASE(TestFindEmpty) {
    HierarchyScopeTracker tracker;

    BOOST_CHECK_THROW(tracker.find({0}), std::runtime_error);
}

// Test find with empty level info
BOOST_AUTO_TEST_CASE(TestFindEmptyLevelInfo) {
    HierarchyScopeTracker tracker;

    tracker.open(100);
    tracker.close(100);

    BOOST_CHECK_THROW(tracker.find({}), std::runtime_error);
}

// Test getDepth
BOOST_AUTO_TEST_CASE(TestGetDepth) {
    HierarchyScopeTracker tracker;

    BOOST_CHECK_EQUAL(tracker.getDepth(), 0);

    tracker.open(100);
    BOOST_CHECK_EQUAL(tracker.getDepth(), 1);

    tracker.open(200);
    BOOST_CHECK_EQUAL(tracker.getDepth(), 2);

    tracker.open(300);
    BOOST_CHECK_EQUAL(tracker.getDepth(), 3);

    tracker.close(300);
    tracker.close(200);
    tracker.close(100);

    // Depth should still be 3 as the structure is preserved
    BOOST_CHECK_EQUAL(tracker.getDepth(), 3);
}

// Test complex hierarchy
BOOST_AUTO_TEST_CASE(TestComplexHierarchy) {
    HierarchyScopeTracker tracker;

    // Build a tree:
    //        1
    //      / | \
    //     2  3  4
    //    /|  |
    //   5 6  7
    //   |
    //   8

    tracker.open(1);
      tracker.open(2);
        tracker.open(5);
          tracker.open(8);
          tracker.close(8);
        tracker.close(5);
        tracker.open(6);
        tracker.close(6);
      tracker.close(2);
      tracker.open(3);
        tracker.open(7);
        tracker.close(7);
      tracker.close(3);
      tracker.open(4);
      tracker.close(4);
    tracker.close(1);

    BOOST_CHECK_EQUAL(tracker.find({0}), 2);
    BOOST_CHECK_EQUAL(tracker.find({1}), 3);
    BOOST_CHECK_EQUAL(tracker.find({2}), 4);
    BOOST_CHECK_EQUAL(tracker.find({0, 0}), 5);
    BOOST_CHECK_EQUAL(tracker.find({0, 1}), 6);
    BOOST_CHECK_EQUAL(tracker.find({1, 0}), 7);
    BOOST_CHECK_EQUAL(tracker.find({0, 0, 0}), 8);

    BOOST_CHECK_EQUAL(tracker.getDepth(), 4);
}

// Test negative index
BOOST_AUTO_TEST_CASE(TestNegativeIndex) {
    HierarchyScopeTracker tracker;

    tracker.open(100);
    tracker.open(200);
    tracker.close(200);
    tracker.close(100);

    BOOST_CHECK_THROW(tracker.find({-1}), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()
