#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <algorithm>
#include <atomic>

#include "engine/matching_engine.h"
#include "dispatch/dispatch_msg.h"
#include "core/order.h"

using namespace std;
using namespace std::chrono;
using namespace dispatch;
using namespace core;

static atomic<uint64_t> gOrderId{1};
inline uint64_t nextOrderId() { return gOrderId.fetch_add(1); }

dispatch::DispatchMsg makeNewOrder(const std::string& sym, Side sd, double price, uint32_t qty) {
    dispatch::DispatchMsg m;
    m.type    = MsgType::NEW_ORDER;
    m.symbol  = sym;
    m.side    = sd;
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

uint64_t pickQ(const vector<uint64_t>& v, double q) {
    if (v.empty()) return 0;
    size_t idx = v.size() * q;
    if (idx >= v.size()) idx = v.size() - 1;
    return v[idx];
}

struct ProducerArgs {
    int tid;
    atomic<bool>* running;
    vector<engine::MatchingEngine*>* engines;
    const string* syms;
    vector<uint64_t>* liveOrders;
    std::mutex* liveOrdersMtx;
    atomic<uint64_t>* pushCounter;
};

void producerThread(ProducerArgs args) {
    std::mt19937 rng(12345 + args.tid);

    std::uniform_int_distribution<int> symDist(0, 3);
    std::uniform_int_distribution<int> cancelDist(0, 99);

    while (args.running->load()) {

        int idx = symDist(rng);
        auto* eng = (*(args.engines))[idx];
        const std::string& sym = args.syms[idx];

        bool isCancel = (cancelDist(rng) < 20); 

        if (!isCancel) {
            Side sd = (rng() % 2 == 0 ? Side::BUY : Side::SELL);

            double price = 100.00 + ((rng() % 100) - 50) * 0.01;

            uint32_t qty = 1 + (rng() % 80);

            auto msg = makeNewOrder(sym, sd, price, qty);
            {
                std::lock_guard<std::mutex> lg(*args.liveOrdersMtx);
                args.liveOrders->push_back(msg.orderId);
            }
            eng->pushInbound(std::move(msg));
        }
        else {
            uint64_t oid = 0;
            {
                std::lock_guard<std::mutex> lg(*args.liveOrdersMtx);
                if (!args.liveOrders->empty()) {
                    oid = args.liveOrders->back();
                    args.liveOrders->pop_back();
                }
            }
            if (oid) {
                auto cancelMsg = makeCancelOrder(sym, oid);
                eng->pushInbound(std::move(cancelMsg));
            }

        }

        args.pushCounter->fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    const int SYMBOLS = 4;
    const string syms[SYMBOLS] = {"MAOTAI", "PINGAN", "ICBC", "TSLA"};

    vector<engine::MatchingEngine*> engines;
    engines.reserve(SYMBOLS);

    for (int i = 0; i < SYMBOLS; i++) {
        auto* eng = new engine::MatchingEngine();
        eng->registerSymbol(syms[i]);
        eng->startEngine();
        engines.push_back(eng);
    }

    cout << "Running Engine Benchmark ...\n";

    vector<uint64_t> liveOrders;
    liveOrders.reserve(1'000'000);
    mutex liveOrdersMtx;

    const int PRODUCER_THREADS = std::thread::hardware_concurrency() / 2;
    cout << "Using " << PRODUCER_THREADS << " producer threads...\n";

    atomic<bool> running{true};
    atomic<uint64_t> pushCount{0};

    vector<std::thread> threads;
    threads.reserve(PRODUCER_THREADS);

    for (int t = 0; t < PRODUCER_THREADS; t++) {
        ProducerArgs args{t, &running, &engines, syms, &liveOrders, &liveOrdersMtx, &pushCount};
        threads.emplace_back(producerThread, args);
    }

    vector<uint64_t> startPop(SYMBOLS);
    uint64_t startPopSum = 0;
    for (int i = 0; i < SYMBOLS; i++) {
        startPop[i] = engines[i]->inboundProcessed_.load(std::memory_order_relaxed);
        startPopSum += startPop[i];
    }

    std::this_thread::sleep_for(1s);
    running.store(false);

    for (auto& th : threads)
        th.join();

    double secs = 1.0;

    vector<uint64_t> endPop(SYMBOLS);
    uint64_t endPopSum = 0;
    for (int i = 0; i < SYMBOLS; i++) {
        endPop[i] = engines[i]->inboundProcessed_.load(std::memory_order_relaxed);
        endPopSum += endPop[i];
    }

    uint64_t engineConsumed = endPopSum - startPopSum;
    int64_t backlog = (int64_t)pushCount - (int64_t)engineConsumed;

    vector<uint64_t> globalLat;

    cout << "\n===== Per-Engine Latency =====\n";
    for (int i = 0; i < SYMBOLS; i++) {
        auto lat = engines[i]->collectLatency();
        std::sort(lat.begin(), lat.end());

        cout << "Engine[" << i << "] samples = " << lat.size() << "\n";
        if (!lat.empty()) {
            cout << "  p50  = " << pickQ(lat,0.50) << " ns\n";
            cout << "  p95  = " << pickQ(lat,0.95) << " ns\n";
            cout << "  p99  = " << pickQ(lat,0.99) << " ns\n";
            cout << "  p999 = " << pickQ(lat,0.999) << " ns\n";
            cout << "  max  = " << lat.back()     << " ns\n";
        }

        globalLat.insert(globalLat.end(), lat.begin(), lat.end());
        cout << endl;
    }

    std::sort(globalLat.begin(), globalLat.end());

    cout << "\n===== Global Latency =====\n";
    cout << "Global samples = " << globalLat.size() << "\n";
    if (!globalLat.empty()) {
        cout << "  p50  = " << pickQ(globalLat,0.50)  << " ns\n";
        cout << "  p95  = " << pickQ(globalLat,0.95)  << " ns\n";
        cout << "  p99  = " << pickQ(globalLat,0.99)  << " ns\n";
        cout << "  p999 = " << pickQ(globalLat,0.999) << " ns\n";
        cout << "  max  = " << globalLat.back()       << " ns\n";
    }

    cout << "\n===== TPS / Backlog =====\n";
    cout << "[Producer TPS]       = " << pushCount.load()         << "\n";
    cout << "[REAL Engine TPS]    = " << engineConsumed           << "\n";
    cout << "[Backlog]            = " << backlog                  << " msgs\n";

    cout << "\n--- Per-Engine TPS ---\n";
    for (int i = 0; i < SYMBOLS; i++) {
        double tps = (endPop[i] - startPop[i]) / secs;
        cout << "Engine[" << i << "] = " << tps
             << " (total=" << endPop[i] << ")\n";
    }

    return 0;
}
