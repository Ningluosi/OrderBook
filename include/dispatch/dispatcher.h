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
    using SendFunc = std::function<bool(int, const std::string&)>;

    Dispatcher(size_t threadCount = 4, size_t queueCapacity = 4096);
    ~Dispatcher();

    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    void setSender(SendFunc sender) { sender_ = std::move(sender); }

    bool pushInbound(DispatchMsg&& msg);
    bool pushOutbound(DispatchMsg&& msg);

    void start();
    void stop();

private:
    void consumerInboundLoop();
    void consumerOutboundLoop(); 


    utils::LockFreeQueue<DispatchMsg> inboundQueue_;
    utils::LockFreeQueue<DispatchMsg> outboundQueue_;
    utils::ThreadPool threadPool_;

    std::thread inboundThread_;
    std::thread outboundThread_;

    std::atomic<bool> running_{false};
    std::thread consumerThread_;
    SendFunc sender_;
};

}
