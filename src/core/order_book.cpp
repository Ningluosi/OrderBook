#include "core/order_book.hpp"
#include <algorithm>

OrderBook::OrderBook(size_t poolSize) : pool_(poolSize) {}

Order* OrderBook::addOrder(Side side, double price, uint32_t qty) {
    Order* order = pool_.allocate();
    order->orderId = nextOrderId_++;
    order->side = side;
    order->price = price;
    order->quantity = qty;

    auto& book = (side == Side::BUY) ? bids_ : asks_;
    auto& level = book[price];
    level.price = price;
    level.append(order);

    orderIndex_[order->orderId] = order;
    updateBestPrices();
    return order;
}

bool OrderBook::cancelOrder(uint64_t orderId) {
    auto it = orderIndex_.find(orderId);
    if (it == orderIndex_.end()) return false;

    Order* order = it->second;
    auto& book = (order->side == Side::BUY) ? bids_ : asks_;
    auto levelIt = book.find(order->price);
    if (levelIt == book.end()) return false;

    levelIt->second.remove(order);
    if (levelIt->second.empty()) book.erase(levelIt);

    pool_.deallocate(order);
    orderIndex_.erase(it);
    updateBestPrices();
    return true;
}

void OrderBook::updateBestPrices() {
    bestBid_ = 0.0;
    bestAsk_ = std::numeric_limits<double>::max();

    if (!bids_.empty()) {
        for (auto& kv : bids_)
            if (kv.first > bestBid_) bestBid_ = kv.first;
    }
    if (!asks_.empty()) {
        for (auto& kv : asks_)
            if (kv.first < bestAsk_) bestAsk_ = kv.first;
    }
}

void OrderBook::printSnapshot(size_t depth) const {
    std::cout << "\n=== ORDER BOOK SNAPSHOT ===" << std::endl;
    std::cout << std::left << std::setw(15) << "BID PRICE"
              << std::setw(15) << "BID QTY"
              << " | "
              << std::setw(15) << "ASK PRICE"
              << std::setw(15) << "ASK QTY" << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;

    std::vector<std::pair<double, uint32_t>> bidVec, askVec;
    for (auto& [price, level] : bids_) {
        uint32_t qty = 0;
        for (auto o = level.head; o; o = o->next) qty += o->quantity;
        bidVec.emplace_back(price, qty);
    }
    for (auto& [price, level] : asks_) {
        uint32_t qty = 0;
        for (auto o = level.head; o; o = o->next) qty += o->quantity;
        askVec.emplace_back(price, qty);
    }

    std::sort(bidVec.begin(), bidVec.end(),
              [](auto& a, auto& b) { return a.first > b.first; });
    std::sort(askVec.begin(), askVec.end(),
              [](auto& a, auto& b) { return a.first < b.first; });

    for (size_t i = 0; i < depth; ++i) {
        std::string bp = (i < bidVec.size()) ? std::to_string(bidVec[i].first) : "";
        std::string bq = (i < bidVec.size()) ? std::to_string(bidVec[i].second) : "";
        std::string ap = (i < askVec.size()) ? std::to_string(askVec[i].first) : "";
        std::string aq = (i < askVec.size()) ? std::to_string(askVec[i].second) : "";

        std::cout << std::left << std::setw(15) << bp
                  << std::setw(15) << bq
                  << " | "
                  << std::setw(15) << ap
                  << std::setw(15) << aq << std::endl;
    }
    std::cout << "=============================\n" << std::endl;
}
