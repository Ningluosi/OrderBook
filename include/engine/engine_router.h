#pragma once
#include "engine/matching_engine.h"
#include <string>
#include <unordered_map>
#include <mutex>

namespace engine {

class EngineRouter {
public:
    static EngineRouter& instance();

    void bindSymbolToEngine(const std::string& symbol, MatchingEngine* engine);

    MatchingEngine* route(const std::string& symbol);

private:
    EngineRouter() = default;

    std::unordered_map<std::string, MatchingEngine*> routeTable_;
    std::mutex mutex_;
};

}
