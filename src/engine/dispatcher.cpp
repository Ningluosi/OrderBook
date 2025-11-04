#include "engine/dispatcher.h"
#include <chrono>

namespace engine {

Dispatcher::Dispatcher(size_t threadCount, size_t queueCapacity)
    : msgQueue_(queueCapacity), threadPool_(threadCount) {}

Dispatcher::~Dispatcher() { stop(); }

void Dispatcher::setSender(SendFunc sender) { sender_ = std::move(sender); }

bool Dispatcher::publish(DispatchMsg msg) {
    return msgQueue_.push(std::move(msg));
}

void Dispatcher::start() {
    if (running_.exchange(true)) return;
    threadPool_.startWorkers();
    consumerThread_ = std::thread([this]{ consumerLoop(); });
}

void Dispatcher::stop() {
    if (!running_.exchange(false)) return;
    if (consumerThread_.joinable()) consumerThread_.join();
    threadPool_.shutdown();
}

void Dispatcher::consumerLoop() {
    using namespace std::chrono_literals;
    DispatchMsg msg;
    while (running_) {
        int idleSpins = 0;
        while (running_ && msgQueue_.pop(msg)) {
            auto task = [m = std::move(msg), send = sender_]() {
                if (send) {
                    send(m.fd, m.payload);
                }
            };
            threadPool_.submitTask(std::move(task));
            idleSpins = 0;
        }
        if (++idleSpins > 64) {
            std::this_thread::sleep_for(50us);
            idleSpins = 0;
        }
    }
}

}
