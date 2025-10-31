#pragma once
#include <cstdint>

enum class Side { BUY, SELL };

struct Order {
    uint64_t orderId = 0;
    Side side;
    double price = 0.0;
    uint32_t quantity = 0;

    Order* next = nullptr;
    Order* prev = nullptr;
};
