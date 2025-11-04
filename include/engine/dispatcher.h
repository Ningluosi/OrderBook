#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <functional>
#include "utils/lockfree_queue.h"
#include "utils/thread_pool.h"

namespace engine {

struct DispatchMsg {
    int fd;
    std::string payload;
};

class Dispatcher {
public:
    using SendFunc = std::function<bool(int, const std::string&)>;

    Dispatcher(size_t threadCount = 4, size_t queueCapacity = 4096);
    ~Dispatcher();

    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    void setSender(SendFunc sender);
    bool publish(DispatchMsg msg);
    void start();
    void stop();

private:
    void consumerLoop();

    utils::LockFreeQueue<DispatchMsg> msgQueue_;
    utils::ThreadPool threadPool_;
    std::atomic<bool> running_{false};
    std::thread consumerThread_;
    SendFunc sender_;
};

}
