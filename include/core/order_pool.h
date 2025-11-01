#pragma once
#include "order.h"
#include <vector>
#include <stack>
#include <stdexcept>

class OrderPool {
public:
    explicit OrderPool(size_t capacity = 100000) {
        pool_.resize(capacity);
        for (size_t i = 0; i < capacity; ++i) freeList_.push(&pool_[i]);
    }

    Order* allocate() {
        if (freeList_.empty()) throw std::runtime_error("OrderPool exhausted");
        Order* order = freeList_.top();
        freeList_.pop();
        return order;
    }

    void deallocate(Order* order) {
        order->next = nullptr;
        order->prev = nullptr;
        freeList_.push(order);
    }

private:
    std::vector<Order> pool_;
    std::stack<Order*> freeList_;
};
