#pragma once
#include "dispatch/dispatch_msg.h"
#include "core/order.h"
#include "utils/logger.h"
#include <nlohmann/json.hpp>

namespace utils {

inline dispatch::DispatchMsg parseMsg(const std::string& data) {
    dispatch::DispatchMsg msg;
    try {
        auto j = nlohmann::json::parse(data);
        std::string type = j.value("type", "UNKNOWN");

        if (type == "NEW_ORDER") msg.type = dispatch::MsgType::NEW_ORDER;
        else if (type == "CANCEL_ORDER") msg.type = dispatch::MsgType::CANCEL_ORDER;
        else if (type == "QUERY_ORDER") msg.type = dispatch::MsgType::QUERY_ORDER;
        else msg.type = dispatch::MsgType::UNKNOWN;

        msg.symbol  = j.value("symbol", "");
        msg.side    = (j.value("side", "BUY") == "BUY") ? core::Side::BUY : core::Side::SELL;
        msg.price   = j.value("price", 0.0);
        msg.qty     = j.value("qty", 0);
        msg.orderId = j.value("orderId", 0);
    } catch (const std::exception& e) {
        LOG_WARN(std::string("[parseMsg] JSON parse error: ") + e.what());
        msg.type = dispatch::MsgType::UNKNOWN;
    }
    return msg;
}

}
