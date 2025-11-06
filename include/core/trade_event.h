#pragma once
#include <string>
#include <cstdint>

namespace core {

struct TradeEvent {
    std::string symbol;
    uint64_t makerOrderId;
    uint64_t takerOrderId;
    double price;
    uint32_t qty;
    uint64_t timestamp;
};

}
