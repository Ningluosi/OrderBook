#include <chrono>
#include "dispatch/dispatcher.h"
#include "engine/engine_router.h"
#include "utils/logger.h"
#include "utils/message_encoder.h"

using namespace std::chrono_literals;
using namespace utils;

namespace dispatch {

Dispatcher::Dispatcher(size_t queueCapacity)
    : readyEngines_(queueCapacity) {}

Dispatcher::~Dispatcher() { stopDispatcher(); }

bool Dispatcher::routeInbound(DispatchMsg&& msg) {
    auto* engine = engine::EngineRouter::instance().route(msg.symbol);
    if (!engine) {
        LOG_WARN("[Dispatcher] No engine found for symbol=" + msg.symbol);
        return false;
    }
    return engine->pushInbound(std::move(msg));
}

void Dispatcher::registerEngine(engine::MatchingEngine* engine) {
    engine->setOutboundCallback([this, engine]() {
        readyEngines_.push(engine);
    });
    LOG_INFO("[Dispatcher] Registered outbound callback for engine");
}

void Dispatcher::startDispatcher() {
    if (running_.exchange(true)) return;
    loopThread_ = std::thread([this] { dispatchLoop(); });
    LOG_INFO("[Dispatcher] Event loop started");
}

void Dispatcher::stopDispatcher() {
    if (!running_.exchange(false)) return;
    if (loopThread_.joinable()) loopThread_.join();
    LOG_INFO("[Dispatcher] Event loop stopped");
}

void Dispatcher::dispatchLoop() {
    engine::MatchingEngine* eng = nullptr;
    int idleSpins = 0;

    while (running_) {
        bool progressed = false;
        while (readyEngines_.pop(eng)) {
            progressed = true;
            processOutbound(*eng);
        }

        if (!progressed) {
            if (++idleSpins > 64) {
                std::this_thread::sleep_for(50us);
                idleSpins = 0;
            }
        } else {
            idleSpins = 0;
        }
    }
}

void Dispatcher::processOutbound(engine::MatchingEngine& eng) {
    if (!sender_) return;

    DispatchMsg msg;
    while (eng.popOutbound(msg)) {
        std::string encoded = encodeMsg(msg);
        sender_(msg.fd, encoded);
    }
}

}
