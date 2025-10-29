#pragma once
#include "order.hpp"
#include <cstdint>

struct PriceLevel {
    double price = 0.0;
    uint32_t totalQty = 0;
    Order* head = nullptr;
    Order* tail = nullptr;

    void append(Order* order) {
        order->next = nullptr;
        order->prev = tail;
        if (tail) tail->next = order;
        tail = order;
        if (!head) head = order;
        totalQty += order->quantity;
    }

    void remove(Order* order) {
        if (order->prev) order->prev->next = order->next;
        if (order->next) order->next->prev = order->prev;
        if (order == head) head = order->next;
        if (order == tail) tail = order->prev;
        totalQty -= order->quantity;
    }

    bool empty() const { return head == nullptr; }
};
