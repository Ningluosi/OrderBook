#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <functional>
#include "utils/lockfree_queue.h"
#include "utils/thread_pool.h"
#include "core/order.h"
#include "dispatch/dispatch_msg.h"

namespace dispatch {

class Dispatcher {
public:
    using SendFunc = std::function<bool(const DispatchMsg&)>;

    explicit Dispatcher(size_t queueCapacity = 1024);
    ~Dispatcher();

    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    void setSender(SendFunc sender) { sender_ = std::move(sender); }

    bool routeInbound(DispatchMsg&& msg);

    void start();
    void stop();

    void registerEngine(engine::MatchingEngine* engine);

private:
    void dispatchLoop();
    void processOutbound(engine::MatchingEngine& eng);

private:
    utils::LockFreeQueue<engine::MatchingEngine*> readyEngines_;
    std::thread loopThread_;
    std::atomic<bool> running_{false};
    SendFunc sender_;
};

}
