#pragma once
#include "core/order.h"
#include <string>

namespace dispatch {

enum class MsgType {
    NEW_ORDER,
    CANCEL_ORDER,
    QUERY_ORDER,
    TRADE_REPORT,
    CANCEL_REPORT,
    ACK,
    UNKNOWN
};

struct DispatchMsg {
    int fd = -1;
    MsgType type = MsgType::UNKNOWN;
    std::string symbol;
    core::Side side = core::Side::BUY;
    double price = 0.0;
    uint32_t qty = 0;
    uint64_t orderId = 0;
    uint64_t clientId = 0;
    std::string payload;
};

}