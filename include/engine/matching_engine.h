#pragma once
#include "core/order_book.h"
#include "dispatch/dispatcher.h"
#include "dispatch/dispatch_msg.h"
#include "utils/logger.h"
#include <string>
#include <memory>

namespace engine {

class MatchingEngine {
public:
    explicit MatchingEngine(dispatch::Dispatcher* dispatcher,
                            std::string symbol,
                            size_t poolSize = 100000)
        : dispatcher_(dispatcher),
        symbol_(std::move(symbol)),
        orderBook_(symbol_, poolSize) {}

    void handleOrderMessage(dispatch::DispatchMsg&& msg);

    const std::string& symbol() const noexcept { return symbol_; }

private:
    dispatch::Dispatcher* dispatcher_{nullptr};
    std::string symbol_;
    core::OrderBook orderBook_;

    void handleNewOrder(const dispatch::DispatchMsg& msg);
    void handleCancelOrder(const dispatch::DispatchMsg& msg);
};

}
