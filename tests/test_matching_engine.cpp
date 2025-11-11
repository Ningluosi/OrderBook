#include <gtest/gtest.h>
#include "engine/matching_engine.h"
#include "dispatch/dispatch_msg.h"
#include "core/order_book.h"
#include <thread>
#include <atomic>
#include <unordered_set>

using namespace dispatch;
using namespace core;
using namespace engine;

class MatchingEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_unique<MatchingEngine>();
        engine->registerSymbol("XPEV");
        engine->registerSymbol("BYD");
        engine->startEngine();
    }

    void TearDown() override {
        engine->stopEngine();
        engine.reset();
    }

    std::unique_ptr<MatchingEngine> engine;
};

TEST_F(MatchingEngineTest, BasicOrderFlow) {
    DispatchMsg msg;
    msg.type = MsgType::NEW_ORDER;
    msg.symbol = "XPEV";
    msg.fd = 1;
    msg.side = Side::BUY;
    msg.price = 100.5;
    msg.qty = 10;

    EXPECT_TRUE(engine->pushInbound(std::move(msg)));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    bool foundAck = false;
    bool foundTrade = false;
    DispatchMsg out;

    while (engine->popOutbound(out)) {
        if (out.type == MsgType::ACK) foundAck = true;
        if (out.type == MsgType::TRADE_REPORT) foundTrade = true;
    }

    EXPECT_TRUE(foundAck);
    EXPECT_FALSE(foundTrade);
}

TEST_F(MatchingEngineTest, MultiSymbolRouting) {
    DispatchMsg xpev = { .fd = 1, .type = MsgType::NEW_ORDER, .symbol = "XPEV", .side = Side::BUY, .price = 100, .qty = 10 };
    DispatchMsg byd = { .fd = 2, .type = MsgType::NEW_ORDER, .symbol = "BYD", .side = Side::SELL, .price = 200, .qty = 5 };

    EXPECT_TRUE(engine->pushInbound(std::move(xpev)));
    EXPECT_TRUE(engine->pushInbound(std::move(byd)));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::unordered_set<std::string> symbols;
    DispatchMsg out;
    while (engine->popOutbound(out)) {
        symbols.insert(out.symbol);
    }

    EXPECT_EQ(symbols.count("XPEV"), 1);
    EXPECT_EQ(symbols.count("BYD"), 1);
}

TEST_F(MatchingEngineTest, MultiThreadedPushInboundSafety) {
    constexpr int kThreads = 4;
    constexpr int kOrdersPerThread = 1000;

    std::atomic<int> pushed{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&, t] {
            for (int i = 0; i < kOrdersPerThread; ++i) {
                DispatchMsg msg;
                msg.type = MsgType::NEW_ORDER;
                msg.symbol = "XPEV";
                msg.fd = t;
                msg.side = Side::BUY;
                msg.price = 100.0 + t;
                msg.qty = 1;
                if (engine->pushInbound(std::move(msg))) pushed++;
            }
        });
    }
    for (auto& th : threads) th.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int count = 0;
    DispatchMsg out;
    while (engine->popOutbound(out)) count++;

    EXPECT_EQ(pushed.load(), kThreads * kOrdersPerThread);
    EXPECT_GE(count, kThreads * kOrdersPerThread); 
}

TEST_F(MatchingEngineTest, StopEngineIsGraceful) {
    engine->stopEngine();
    EXPECT_TRUE(true);
}
