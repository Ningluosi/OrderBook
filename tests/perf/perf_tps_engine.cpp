#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include "engine/matching_engine.h"
#include "dispatch/dispatch_msg.h"
#include "core/order.h"

using namespace std::chrono;
using namespace dispatch;
using namespace core;

static uint64_t nextOrderId() {
    static uint64_t id = 1;
    return id++;
}

dispatch::DispatchMsg makeNewOrder(const std::string& sym, Side side, double price, uint32_t qty) {
    dispatch::DispatchMsg m;
    m.type    = MsgType::NEW_ORDER;
    m.symbol  = sym;
    m.side    = side;
    m.price   = price;
    m.qty     = qty;
    m.fd      = -1;
    m.orderId = nextOrderId();
    return m;
}

dispatch::DispatchMsg makeCancelOrder(const std::string& sym, uint64_t orderId) {
    dispatch::DispatchMsg m;
    m.type    = MsgType::CANCEL_ORDER;
    m.symbol  = sym;
    m.orderId = orderId;
    m.fd      = -1;
    return m;
}

int main() {
    const int SYMBOLS = 4;
    const std::string syms[SYMBOLS] = {"MAOTAI", "PINGAN", "ICBC", "TSLA"};

    std::vector<engine::MatchingEngine*> engines;
    engines.reserve(SYMBOLS);
    for (int i = 0; i < SYMBOLS; i++) {
        auto* eng = new engine::MatchingEngine();
        eng->registerSymbol(syms[i]);
        eng->startEngine();
        engines.push_back(eng);
    }

    std::cout << "Running Real Engine TPS test...\n";

    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> sid(0, SYMBOLS - 1);
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_int_distribution<int> opDist(0, 10);

    std::vector<uint64_t> liveOrders;
    liveOrders.reserve(1'000'000);

    uint64_t pushCount = 0;

    uint64_t startPopSum = 0;
    for (int i = 0; i < SYMBOLS; ++i) {
        startPopSum += engines[i]->inboundProcessed_.load(std::memory_order_relaxed);
    }

    auto start = steady_clock::now();

    while (steady_clock::now() - start < 1s) {

        int sidx = sid(rng);
        auto* eng = engines[sidx];
        const std::string& sym = syms[sidx];

        int op = opDist(rng);

        if (op < 8) {
            Side sd = (sideDist(rng) == 0 ? Side::BUY : Side::SELL);
            double price = 100.0 + (rng() % 2000) * 0.01;
            uint32_t qty = 1 + (rng() % 50);

            auto msg = makeNewOrder(sym, sd, price, qty);
            liveOrders.push_back(msg.orderId);
            eng->pushInbound(std::move(msg));
        } else {
            if (!liveOrders.empty()) {
                uint64_t oid = liveOrders.back();
                liveOrders.pop_back();

                auto cancelMsg = makeCancelOrder(sym, oid);
                eng->pushInbound(std::move(cancelMsg));
            }
        }

        ++pushCount;
    }

    auto end = steady_clock::now();
    double secs = duration_cast<duration<double>>(end - start).count();

    uint64_t endPopSum = 0;
    for (int i = 0; i < SYMBOLS; ++i) {
        endPopSum += engines[i]->inboundProcessed_.load(std::memory_order_relaxed);
    }

    uint64_t engineConsumed = endPopSum - startPopSum;

    std::cout << "[Info] Producer Push Duration = " << secs << " sec\n";
    std::cout << "[Info] Producer Push Count    = " << pushCount << "\n";
    std::cout << "[Producer TPS]               = " << static_cast<uint64_t>(pushCount / secs) << " ops/sec\n";
    std::cout << "[REAL Engine TPS]            = " << static_cast<uint64_t>(engineConsumed / secs) << " ops/sec\n";

    for (int i = 0; i < SYMBOLS; ++i) {
        uint64_t c = engines[i]->inboundProcessed_.load(std::memory_order_relaxed);
        std::cout << "Engine[" << i << "] Total Processed = " << c << std::endl;
    }

    return 0;
}
