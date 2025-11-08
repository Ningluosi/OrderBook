#include "dispatch/dispatcher.h"
#include "engine/engine_router.h"
#include <chrono>

using namespace std::chrono_literals;
namespace dispatch {

Dispatcher::Dispatcher(size_t threadCount, size_t queueCapacity)
    : inboundQueue_(queueCapacity),
      outboundQueue_(queueCapacity),
      threadPool_(threadCount) {}

Dispatcher::~Dispatcher() { stop(); }

bool Dispatcher::pushInbound(DispatchMsg&& msg) {
    return inboundQueue_.push(std::move(msg));
}

bool Dispatcher::pushOutbound(DispatchMsg&& msg) {
    return outboundQueue_.push(std::move(msg));
}

void Dispatcher::start() {
    if (running_.exchange(true)) return;
    threadPool_.startWorkers();

    inboundThread_  = std::thread([this]{ consumerInboundLoop(); });
    outboundThread_ = std::thread([this]{ consumerOutboundLoop(); });
}

void Dispatcher::stop() {
    if (!running_.exchange(false)) return;
    if (inboundThread_.joinable())  inboundThread_.join();
    if (outboundThread_.joinable()) outboundThread_.join();
    threadPool_.shutdown();
}

void Dispatcher::consumerInboundLoop() {
    DispatchMsg msg;
    while (running_) {
        int idleSpins = 0;
        while (running_ && inboundQueue_.pop(msg)) {
            auto task = [m = std::move(msg)]() mutable {
                auto* engine = engine::EngineRouter::instance().route(m.symbol);
                if (engine) {
                    engine->handleOrderMessage(std::move(m));
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

void Dispatcher::consumerOutboundLoop() {
    DispatchMsg msg;
    while (running_) {
        int idleSpins = 0;
        while (running_ && outboundQueue_.pop(msg)) {
            if (sender_) {
                sender_(msg.fd, msg.payload);
            }
            idleSpins = 0;
        }
        if (++idleSpins > 64) {
            std::this_thread::sleep_for(50us);
            idleSpins = 0;
        }
    }
}

}
