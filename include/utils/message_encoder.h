#pragma once
#include "dispatch/dispatch_msg.h"
#include <nlohmann/json.hpp>

namespace utils {

inline std::string encodeMsg(const dispatch::DispatchMsg& msg) {
    nlohmann::json j;
    j["type"] = [msg]() {
        switch (msg.type) {
            case dispatch::MsgType::TRADE_REPORT:  return "TRADE_REPORT";
            case dispatch::MsgType::CANCEL_REPORT: return "CANCEL_REPORT";
            case dispatch::MsgType::ACK:           return "ACK";
            default:                               return "UNKNOWN";
        }
    }();

    if (!msg.symbol.empty()) j["symbol"] = msg.symbol;
    if (msg.price > 0)       j["price"]  = msg.price;
    if (msg.qty > 0)         j["qty"]    = msg.qty;
    if (msg.orderId > 0)     j["orderId"] = msg.orderId;
    if (msg.makerId > 0)     j["makerId"] = msg.makerId;
    if (msg.takerId > 0)     j["takerId"] = msg.takerId;
    if (!msg.status.empty()) j["status"]  = msg.status;
    if (!msg.msg.empty())    j["msg"]     = msg.msg;

    return j.dump();
}

}