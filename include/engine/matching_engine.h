#pragma once
#include "core/order_book.h"
#include "engine/dispatcher.h"
#include "utils/logger.h"
#include <string>
#include <memory>

namespace engine {

class MatchingEngine {
public:
    explicit MatchingEngine(Dispatcher* dispatcher,
                            std::string symbol,
                            size_t poolSize = 100000)
        : dispatcher_(dispatcher),
        symbol_(std::move(symbol)),
        orderBook_(symbol_, poolSize) {}

    void handleOrderMessage(DispatchMsg&& msg);

    const std::string& symbol() const noexcept { return symbol_; }

private:
    Dispatcher* dispatcher_{nullptr};
    std::string symbol_;
    core::OrderBook orderBook_;

    void handleNewOrder(const DispatchMsg& msg);
    void handleCancelOrder(const DispatchMsg& msg);
};

}
