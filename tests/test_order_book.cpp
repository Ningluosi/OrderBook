#include "core/order_book.hpp"
#include <iostream>
#include <cassert>

void testAddAndCancel() {
    OrderBook book(1000);
    auto* o1 = book.addOrder(Side::BUY, 100.5, 10);
    auto* o2 = book.addOrder(Side::BUY, 99.8, 20);
    auto* o3 = book.addOrder(Side::SELL, 101.2, 15);

    std::cout << "[TEST] After adding 3 orders:" << std::endl;
    book.printSnapshot();

    assert(o1 && o2 && o3);
    assert(book.cancelOrder(o2->orderId));
    std::cout << "[TEST] After canceling orderId=" << o2->orderId << std::endl;
    book.printSnapshot();
}

int main() {
    std::cout << "Running OrderBook Tests..." << std::endl;
    testAddAndCancel();
    std::cout << "All tests passed." << std::endl;
    return 0;
}
