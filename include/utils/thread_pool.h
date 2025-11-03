#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <queue>
#include <mutex>

namespace utils {

class ThreadPool {
public:
    explicit ThreadPool(size_t nThreads = std::thread::hardware_concurrency());
    ~ThreadPool();

    void startWorkers();
    void shutdown();
    bool submitTask(std::function<void()> fn);

private:
    void runWorkerLoop();

    size_t nThreads_;
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> running_{false};
};

}
