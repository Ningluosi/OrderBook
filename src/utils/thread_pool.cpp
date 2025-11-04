#include "utils/thread_pool.h"

namespace utils {

ThreadPool::ThreadPool(size_t nThreads)
    : nThreads_(nThreads ? nThreads : 1) {
    workers_.reserve(nThreads_);
}


ThreadPool::~ThreadPool() { shutdown(); }

void ThreadPool::startWorkers() {
    if (poolRunning_.exchange(true)) return;
    for (auto &t : workers_) {
        t = std::thread([this]{ runWorkerLoop(); });
    }
}

void ThreadPool::shutdown() {
    if (!poolRunning_.exchange(false)) return;
    cv_.notify_all();
    for (auto &t : workers_) {
        if (t.joinable()) t.join();
    }
}

bool ThreadPool::submitTask(std::function<void()> fn) {
    if (!poolRunning_) return false;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        tasks_.push(std::move(fn));
    }
    cv_.notify_one();
    return true;
}

void ThreadPool::runWorkerLoop() {
    while (true) {
        std::function<void()> taskFn;
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [&]{ return !poolRunning_ || !tasks_.empty(); });
            if (!poolRunning_ && tasks_.empty()) break;
            taskFn = std::move(tasks_.front());
            tasks_.pop();
        }

        try {
            if (taskFn) taskFn();
        } catch (const std::exception& e) {
            LOG_ERROR(std::string("[ThreadPool] Task threw exception: ") + e.what());
        } catch (...) {
            LOG_ERROR("[ThreadPool] Unknown exception in task");
        }
    }
}

}
