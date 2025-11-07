#include "engine/engine_router.h"
#include "utils/logger.h"

using namespace utils;

namespace engine {

EngineRouter& EngineRouter::instance() {
    static EngineRouter router;
    return router;
}

void EngineRouter::registerEngine(const std::string& symbol, MatchingEngine* engine) {
    std::lock_guard<std::mutex> lock(mutex_);
    routeTable_[symbol] = engine;
}

MatchingEngine* EngineRouter::route(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = routeTable_.find(symbol);
    if (it != routeTable_.end()) {
        return it->second;
    }
    LOG_WARN("[EngineRouter] No engine found for symbol=" + symbol);
    return nullptr;
}

}
