#pragma once
#include "core/order_pool.h"
#include "core/price_level.h"
#include "core/trade_event.h"
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>

namespace core {

class OrderBook {
public:
    explicit OrderBook(const std::string& symbol, size_t poolSize = 100000);

    Order* addOrder(Side side, double price, uint32_t qty);
    bool cancelOrder(uint64_t orderId);
    void matchOrder(Side side, double price, uint32_t qty);

    void printSnapshot(size_t depth = 5) const;

    const std::unordered_map<double, PriceLevel>& bids() const noexcept { return bids_; }
    const std::unordered_map<double, PriceLevel>& asks() const noexcept { return asks_; }
    const std::unordered_map<uint64_t, Order*>& orderIndex() const noexcept { return orderIndex_; }

    double bestBid() const noexcept { return bestBid_; }
    double bestAsk() const noexcept { return bestAsk_; }

    const std::vector<TradeEvent>& getTradeEvents() const noexcept { return tradeEvents_; }
    void clearTradeEvents() noexcept { tradeEvents_.clear(); }

private:
    std::string symbol_;
    uint64_t nextOrderId_ = 1;
    OrderPool orderPool_;

    std::unordered_map<double, PriceLevel> bids_;
    std::unordered_map<double, PriceLevel> asks_;
    std::unordered_map<uint64_t, Order*> orderIndex_;
    std::vector<TradeEvent> tradeEvents_;

    double bestBid_ = 0.0;
    double bestAsk_ = std::numeric_limits<double>::max();

    void updateBestPrices();
    void executeTrade(Order* taker, Order* maker, uint32_t tradedQty, double tradePrice);
};

}