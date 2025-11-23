#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <algorithm>
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

uint64_t pickQ(const std::vector<uint64_t>& v, double q) {
    if (v.empty()) return 0;
    size_t idx = v.size() * q;
    if (idx >= v.size()) idx = v.size() - 1;
    return v[idx];
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

    std::cout << "Running Engine Benchmark (TPS + Backlog + Latency)...\n";

    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> sid(0, SYMBOLS - 1);
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_int_distribution<int> opDist(0, 10);

    std::vector<uint64_t> liveOrders;
    liveOrders.reserve(1'000'000);

    uint64_t pushCount = 0;

    uint64_t startPopSum = 0;
    std::vector<uint64_t> startPopEach(SYMBOLS);
    for (int i = 0; i < SYMBOLS; ++i) {
        startPopEach[i] = engines[i]->inboundProcessed_.load(std::memory_order_relaxed);
        startPopSum += startPopEach[i];
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
    std::vector<uint64_t> endPopEach(SYMBOLS);
    for (int i = 0; i < SYMBOLS; ++i) {
        endPopEach[i] = engines[i]->inboundProcessed_.load(std::memory_order_relaxed);
        endPopSum += endPopEach[i];
    }

    uint64_t engineConsumed = endPopSum - startPopSum;
    int64_t backlog = (int64_t)pushCount - (int64_t)engineConsumed;

    std::vector<uint64_t> globalLat;

    std::cout << "\n===== Per-Engine Latency =====\n";
    for (int i = 0; i < SYMBOLS; ++i) {
        auto lat = engines[i]->collectLatency();
        std::sort(lat.begin(), lat.end());

        std::cout << "Engine[" << i << "] samples = " << lat.size() << "\n";

        if (!lat.empty()) {
            std::cout << "  p50  = " << pickQ(lat,0.50)  << " ns\n";
            std::cout << "  p95  = " << pickQ(lat,0.95)  << " ns\n";
            std::cout << "  p99  = " << pickQ(lat,0.99)  << " ns\n";
            std::cout << "  p999 = " << pickQ(lat,0.999) << " ns\n";
            std::cout << "  max  = " << lat.back()       << " ns\n";
        }

        globalLat.insert(globalLat.end(), lat.begin(), lat.end());
        std::cout << "\n";
    }

    std::sort(globalLat.begin(), globalLat.end());

    std::cout << "\n===== Global Latency (Merged All Engines) =====\n";
    std::cout << "Global samples = " << globalLat.size() << "\n";

    if (!globalLat.empty()) {
        std::cout << "  p50  = " << pickQ(globalLat,0.50)  << " ns\n";
        std::cout << "  p95  = " << pickQ(globalLat,0.95)  << " ns\n";
        std::cout << "  p99  = " << pickQ(globalLat,0.99)  << " ns\n";
        std::cout << "  p999 = " << pickQ(globalLat,0.999) << " ns\n";
        std::cout << "  max  = " << globalLat.back()       << " ns\n";
    }

    std::cout << "\n===== TPS / Backlog =====\n";
    std::cout << "[Producer Push Duration] = " << secs << " sec\n";
    std::cout << "[Producer Push Count]    = " << pushCount << "\n";
    std::cout << "[Producer TPS]           = " << (uint64_t)(pushCount / secs) << "\n";
    std::cout << "[REAL Engine TPS]        = " << (uint64_t)(engineConsumed / secs) << "\n";
    std::cout << "[Backlog]                = " << backlog << " msgs\n";

    std::cout << "\n--- Per-Engine TPS ---\n";
    for (int i = 0; i < SYMBOLS; ++i) {
        double tps = (endPopEach[i] - startPopEach[i]) / secs;
        std::cout << "Engine[" << i << "] = " << tps 
                  << " (total " << endPopEach[i] << ")\n";
    }

    return 0;
}
