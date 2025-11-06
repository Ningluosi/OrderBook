#include "engine/matching_engine.h"

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
            utils::LOG_WARN("[MatchingEngine] Unknown message type");
            break;
    }
}

void MatchingEngine::handleCancelOrder(const DispatchMsg& msg) {
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
    } else {
        utils::LOG_WARN("[MatchingEngine] dispatcher_ is null, CANCEL_REPORT not sent");
    }
}

void MatchingEngine::handleNewOrder(const DispatchMsg& msg) {
    orderBook_.matchOrder(msg.side, msg.price, msg.qty);

    if (dispatcher_) {
        for (const auto& evt : orderBook_.getTradeEvents()) {
            DispatchMsg trade;
            trade.type = MsgType::TRADE_REPORT;
            trade.payload = R"({"type":"TRADE","price":)"
                + std::to_string(evt.price)
                + R"(,"qty":)" + std::to_string(evt.qty)
                + R"(,"makerId":)" + std::to_string(evt.makerOrderId)
                + R"(,"takerId":)" + std::to_string(evt.takerOrderId) + "}";
            dispatcher_->pushOutbound(std::move(trade));
        }
    } else {
        utils::LOG_WARN("[MatchingEngine] dispatcher_ is null, TradeEvent not sent");
    }

    orderBook_.clearTradeEvents();
}

}