#pragma once
#include "order_pool.hpp"
#include "price_level.hpp"
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>

class OrderBook {
public:
    explicit OrderBook(size_t poolSize = 100000);

    // 下单与撤单
    Order* addOrder(Side side, double price, uint32_t qty);
    bool cancelOrder(uint64_t orderId);

    // 打印快照
    void printSnapshot(size_t depth = 5) const;

private:
    uint64_t nextOrderId_ = 1;
    OrderPool pool_;

    std::unordered_map<double, PriceLevel> bids_;
    std::unordered_map<double, PriceLevel> asks_;
    std::unordered_map<uint64_t, Order*> orderIndex_;

    double bestBid_ = 0.0;
    double bestAsk_ = std::numeric_limits<double>::max();

    void updateBestPrices();
};
