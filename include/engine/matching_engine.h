#pragma once
#include <unordered_map>
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <boost/lockfree/spsc_queue.hpp>
#include "core/order_book.h"
#include "dispatch/dispatch_msg.h"
#include "concurrentqueue/concurrentqueue.h"
#include "utils/logger.h"

namespace engine {

class MatchingEngine {
public:
    static constexpr size_t LAT_BUF = 1 << 20;

    explicit MatchingEngine(size_t inboundCap = 4096,
                            size_t outboundCap = 4096)
        : inboundQueue_(inboundCap),
          outboundQueue_(outboundCap) {}

    ~MatchingEngine();

    void startEngine();
    void stopEngine();

    bool registerSymbol(const std::string& symbol, size_t poolSize = 100000);

    bool pushInbound(dispatch::DispatchMsg&& msg);
    bool popOutbound(dispatch::DispatchMsg& out);
    bool pushOutbound(const dispatch::DispatchMsg&& msg);
    void handleOrderMessage(dispatch::DispatchMsg&& msg);
    void setOutboundCallback(std::function<void()> cb);

    void recordLatency(uint64_t ns);
    std::vector<uint64_t> collectLatency() const;
    std::atomic<uint64_t> inboundProcessed_{0};

private:
    void matchingLoop();
    void handleNewOrder(const dispatch::DispatchMsg& msg, core::OrderBook& ob);
    void handleCancelOrder(const dispatch::DispatchMsg& msg, core::OrderBook& ob);

private:
    std::unordered_map<std::string, core::OrderBook> orderBooks_;
    moodycamel::ConcurrentQueue<dispatch::DispatchMsg> inboundQueue_;
    moodycamel::ConcurrentQueue<dispatch::DispatchMsg> outboundQueue_;
    std::function<void()> outboundReadyCallback_;
    std::thread matchingThread_;
    std::atomic<bool> running_{false};
    mutable boost::lockfree::spsc_queue< uint64_t, boost::lockfree::capacity<LAT_BUF>> latencyQueue_;
};

}
