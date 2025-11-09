#include "utils/thread_pool.h"
#include "utils/logger.h"

namespace utils {

ThreadPool::ThreadPool(size_t nThreads)
    : nThreads_(nThreads ? nThreads : 1) {}


ThreadPool::~ThreadPool() { shutdown(); }

void ThreadPool::startWorkers() {
    if (poolRunning_.exchange(true)) return;
    for (size_t i = 0; i < nThreads_; ++i) {
        workers_.emplace_back([this, i] { runWorkerLoop(i); });
    }
}

void ThreadPool::shutdown() {
    if (!poolRunning_.exchange(false)) return;
    cv_.notify_all();
    for (auto &t : workers_) {
        if (t.joinable()) t.join();
    }
}

void ThreadPool::runWorkerLoop(size_t id) {
    LOG_INFO("[ThreadPool] Worker #" + std::to_string(id) + " started");
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
            LOG_ERROR("[ThreadPool] Worker #" + std::to_string(id) + " exception: " + e.what());
        } catch (...) {
            LOG_ERROR("[ThreadPool] Worker #" + std::to_string(id) + " unknown exception");
        }
    }
}

}
