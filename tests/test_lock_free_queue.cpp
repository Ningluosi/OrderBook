#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include "utils/lockfree_queue.h"

using namespace utils;

TEST(LockFreeQueueTest, BasicPushPop) {
    LockFreeQueue<int> q(8);
    for (int i = 0; i < 5; ++i)
        EXPECT_TRUE(q.push(i));

    for (int i = 0; i < 5; ++i) {
        int out = -1;
        EXPECT_TRUE(q.pop(out));
        EXPECT_EQ(out, i);
    }
}

TEST(LockFreeQueueTest, PopFromEmpty) {
    LockFreeQueue<int> q(4);
    int out = 0;
    EXPECT_FALSE(q.pop(out));
}

TEST(LockFreeQueueTest, PushToFullQueue) {
    LockFreeQueue<int> q(4);
    for (int i = 0; i < 4; ++i)
        EXPECT_TRUE(q.push(i));
    EXPECT_FALSE(q.push(100)); 
}

TEST(LockFreeQueueTest, CircularReuse) {
    LockFreeQueue<int> q(4);

    for (int i = 0; i < 3; ++i) q.push(i);
    int x;
    for (int i = 0; i < 2; ++i) q.pop(x);

    EXPECT_TRUE(q.push(100));
    EXPECT_TRUE(q.push(200));

    std::vector<int> results;
    while (q.pop(x)) results.push_back(x);
    EXPECT_EQ(results.size(), 3);
    EXPECT_TRUE((results == std::vector<int>{2, 100, 200}));
}


