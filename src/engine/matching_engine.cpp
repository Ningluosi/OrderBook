#include "engine/matching_engine.h"

using namespace utils;
using namespace dispatch;

namespace engine {

void MatchingEngine::handleOrderMessage(DispatchMsg&& msg) {
    switch (msg.type) {
        case MsgType::NEW_ORDER:
            handleNewOrder(msg);
            break;

        case MsgType::CANCEL_ORDER:
            handleCancelOrder(msg);
            break;

        default:
            LOG_WARN("[MatchingEngine] Unknown message type");
            break;
    }
}

void MatchingEngine::handleCancelOrder(const DispatchMsg& msg) {
    LOG_INFO("[MatchingEngine][" + symbol_ + "] CANCEL_REQ"
             " orderId=" + std::to_string(msg.orderId) +
             " fd=" + std::to_string(msg.fd));

    bool ok = orderBook_.cancelOrder(msg.orderId);

    DispatchMsg resp;
    resp.fd = msg.fd;
    resp.type = MsgType::CANCEL_REPORT;
    resp.symbol = msg.symbol;
    resp.payload = ok
        ? R"({"status":"CANCEL_OK","orderId":)" + std::to_string(msg.orderId) + "}"
        : R"({"status":"NOT_FOUND","orderId":)" + std::to_string(msg.orderId) + "}";

    if (dispatcher_) {
        dispatcher_->pushOutbound(std::move(resp));
        LOG_INFO("[MatchingEngine][" + symbol_ + "] CANCEL_REPORT sent to fd="
                 + std::to_string(msg.fd) +
                 (ok ? " (CANCEL_OK)" : " (NOT_FOUND)"));
    } else {
        LOG_WARN("[MatchingEngine] dispatcher_ is null, CANCEL_REPORT not sent");
    }
}

void MatchingEngine::handleNewOrder(const DispatchMsg& msg) {
    LOG_INFO("[MatchingEngine][" + symbol_ + "] NEW_ORDER"
             " side=" + std::string(msg.side == core::Side::BUY ? "BUY" : "SELL") +
             " price=" + std::to_string(msg.price) +
             " qty=" + std::to_string(msg.qty) +
             " fd=" + std::to_string(msg.fd));


    orderBook_.matchOrder(msg.side, msg.price, msg.qty);

    if (dispatcher_) {
        for (const auto& evt : orderBook_.getTradeEvents()) {
            DispatchMsg trade;
            trade.type = MsgType::TRADE_REPORT;
            trade.fd = msg.fd;
            trade.payload = R"({"type":"TRADE","price":)"
                + std::to_string(evt.price)
                + R"(,"qty":)" + std::to_string(evt.qty)
                + R"(,"makerId":)" + std::to_string(evt.makerOrderId)
                + R"(,"takerId":)" + std::to_string(evt.takerOrderId) + "}";
            dispatcher_->pushOutbound(std::move(trade));

            LOG_INFO("[MatchingEngine][" + symbol_ + "] TRADE_REPORT sent:"
                     " price=" + std::to_string(evt.price) +
                     " qty=" + std::to_string(evt.qty) +
                     " makerId=" + std::to_string(evt.makerOrderId) +
                     " takerId=" + std::to_string(evt.takerOrderId));
        }
    } else {
        LOG_WARN("[MatchingEngine] dispatcher_ is null, TradeEvent not sent");
    }

    orderBook_.clearTradeEvents();
}

}