#include "engine/matching_engine.h"

using namespace utils;
using namespace dispatch;

namespace engine {

MatchingEngine::~MatchingEngine() { stopEngine(); }

bool MatchingEngine::registerSymbol(const std::string& symbol, size_t poolSize) {
    auto [it, ok] = orderBooks_.try_emplace(symbol, symbol, poolSize);
    if (ok) {
        LOG_INFO("[MatchingEngine] registered symbol=" + symbol);
    } else {
        LOG_WARN("[MatchingEngine] duplicate symbol=" + symbol);
    }
    return ok;
}

void MatchingEngine::startEngine() {
    if (running_.exchange(true)) return;
    matchingThread_ = std::thread([this]{ matchingLoop(); });
}

void MatchingEngine::stopEngine() {
    if (!running_.exchange(false)) return;
    if (matchingThread_.joinable()) matchingThread_.join();
}

bool MatchingEngine::pushInbound(DispatchMsg&& msg) {
    return inboundQueue_.push(std::move(msg));
}

bool MatchingEngine::popOutbound(DispatchMsg& out) {
    return outboundQueue_.pop(out);
}

void MatchingEngine::matchingLoop() {
    LOG_INFO("[MatchingEngine] thread started, symbols=" + std::to_string(orderBooks_.size()));
    DispatchMsg msg;
    int idleSpins = 0;

    while (running_) {
        bool progressed = false;
        while (inboundQueue_.pop(msg)) {
            progressed = true;
            handleOrderMessage(std::move(msg));
        }

        if (!progressed) {
            if (++idleSpins > 64) {
                std::this_thread::sleep_for(std::chrono::microseconds(50));
                idleSpins = 0;
            }
        } else {
            idleSpins = 0;
        }
    }

    LOG_INFO("[MatchingEngine] thread stopped");
}

void MatchingEngine::handleOrderMessage(DispatchMsg&& msg) {
    auto it = orderBooks_.find(msg.symbol);
    if (it == orderBooks_.end()) {
        LOG_WARN("[MatchingEngine] unknown symbol=" + msg.symbol);

        DispatchMsg err;
        err.fd     = msg.fd;
        err.type   = MsgType::UNKNOWN;
        err.symbol = msg.symbol;
        err.status = "UNKNOWN_SYMBOL";
        outboundQueue_.push(std::move(err));
        return;
    }
    auto& ob = it->second;

    switch (msg.type) {
        case MsgType::NEW_ORDER:
            handleNewOrder(msg, ob);
            break;
        case MsgType::CANCEL_ORDER:
            handleCancelOrder(msg, ob);
            break;
        default: {
            LOG_WARN("[MatchingEngine][" + msg.symbol + "] Unknown msg type");
            DispatchMsg err;
            err.fd     = msg.fd;
            err.type   = MsgType::UNKNOWN;
            err.symbol = msg.symbol;
            err.status = "UNKNOWN_MSGTYPE";
            outboundQueue_.push(std::move(err));
            break;
        }
    }
}

void MatchingEngine::handleCancelOrder(const DispatchMsg& msg, core::OrderBook& ob) {
    LOG_INFO("[MatchingEngine][" + ob.symbol() + "] CANCEL_REQ"
             " orderId=" + std::to_string(msg.orderId) +
             " fd=" + std::to_string(msg.fd));

    bool ok = ob.cancelOrder(msg.orderId);

    DispatchMsg resp;
    resp.fd = msg.fd;
    resp.type = MsgType::CANCEL_REPORT;
    resp.symbol = msg.symbol;
    resp.orderId= msg.orderId;
    resp.status = ok ? "CANCEL_OK" : "NOT_FOUND";

    outboundQueue_.push(std::move(resp));

    LOG_INFO("[MatchingEngine][" + ob.symbol() + "] CANCEL_REPORT fd="
             + std::to_string(msg.fd) + " status=" + (ok ? "OK" : "NOT_FOUND"));
}

void MatchingEngine::handleNewOrder(const DispatchMsg& msg, core::OrderBook& ob) {
    LOG_INFO("[MatchingEngine][" + ob.symbol() + "] NEW_ORDER"
             " side=" + std::string(msg.side == core::Side::BUY ? "BUY" : "SELL") +
             " price=" + std::to_string(msg.price) +
             " qty=" + std::to_string(msg.qty) +
             " fd=" + std::to_string(msg.fd));

    {
        DispatchMsg ack;
        ack.fd     = msg.fd;
        ack.type   = MsgType::ACK;
        ack.symbol = msg.symbol;
        ack.status = "RECEIVED";
        outboundQueue_.push(std::move(ack));
    }

    ob.matchOrder(msg.side, msg.price, msg.qty);

    for (const auto& evt : ob.getTradeEvents()) {
        DispatchMsg trade;
        trade.type    = MsgType::TRADE_REPORT;
        trade.fd      = msg.fd;
        trade.symbol  = msg.symbol;
        trade.price   = evt.price;
        trade.qty     = evt.qty;
        trade.makerId = evt.makerOrderId;
        trade.takerId = evt.takerOrderId;
        outboundQueue_.push(std::move(trade));

        LOG_INFO("[MatchingEngine][" + ob.symbol() + "] TRADE_REPORT"
                 " px=" + std::to_string(evt.price) +
                 " qty=" + std::to_string(evt.qty) +
                 " maker=" + std::to_string(evt.makerOrderId) +
                 " taker=" + std::to_string(evt.takerOrderId));
    }

    ob.clearTradeEvents();
}

}