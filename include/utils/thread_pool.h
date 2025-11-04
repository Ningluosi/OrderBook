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

    template<typename F>
    bool submitTask(F&& fn) {
        if (!poolRunning_) return false;
        {
            std::lock_guard<std::mutex> lock(mtx_);
            tasks_.emplace(std::forward<F>(fn));
        }
        cv_.notify_one();
        return true;
    }

private:
    void runWorkerLoop();

    size_t nThreads_;
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> poolRunning_{false};
};

}
