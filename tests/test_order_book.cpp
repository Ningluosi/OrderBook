#include <gtest/gtest.h>
#include "core/order_book.hpp"

class OrderBookTest : public ::testing::Test {
protected:
    OrderBook book{10000};

    void SetUp() override {
        book.addOrder(Side::SELL, 101.0, 10);
        book.addOrder(Side::SELL, 102.0, 10);
        book.addOrder(Side::BUY,  99.0,  10);
        book.addOrder(Side::BUY,  98.0,  10);
    }
};


TEST_F(OrderBookTest, FullMatchBuyOrder) {
    book.matchOrder(Side::BUY, 102.0, 25);

    EXPECT_TRUE(book.asks().empty());
    EXPECT_EQ(book.bids().size(), 3);
}

TEST_F(OrderBookTest, PartialMatchBuyOrder) {
    book.matchOrder(Side::BUY, 101.0, 5);

    auto it = book.asks().find(101.0);
    ASSERT_NE(it, book.asks().end());
    EXPECT_EQ(it->second.totalQty, 5);
}

TEST_F(OrderBookTest, AddBuyOrderBelowBestAsk) {
    book.matchOrder(Side::BUY, 100.0, 8);

    auto it = book.bids().find(100.0);
    ASSERT_NE(it, book.bids().end());
    EXPECT_EQ(it->second.totalQty, 8);
}

TEST_F(OrderBookTest, FIFOWithinSamePriceLevel) {
    auto* o1 = book.addOrder(Side::BUY, 100.5, 10);
    auto* o2 = book.addOrder(Side::BUY, 100.5, 20);

    book.matchOrder(Side::SELL, 100.5, 15);

    EXPECT_EQ(o1->quantity, 0);
    EXPECT_EQ(o2->quantity, 15);
}

TEST_F(OrderBookTest, BestPriceUpdatesAfterMatch) {
    book.matchOrder(Side::BUY, 101.0, 10);

    EXPECT_DOUBLE_EQ(book.bestAsk(), 102.0);
    EXPECT_DOUBLE_EQ(book.bestBid(), 99.0);
}
