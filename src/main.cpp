#include "net/epoll_reactor.h"
#include "net/tcp_server.h"
#include "dispatch/dispatcher.h"
#include "engine/engine_router.h"
#include "engine/matching_engine.h"
#include "utils/message_parser.h"
#include "utils/message_encoder.h"
#include "utils/logger.h"

using namespace net;
using namespace engine;
using namespace utils;
using namespace dispatch;


int main() {
    LOG_INFO("=== OrderBook System Starting ===");

    net::EpollReactor reactor;

    Dispatcher dispatcher(1024);
    dispatcher.startDispatcher();

    auto* engine = new MatchingEngine();

    engine->registerSymbol("AAPL",   100000); 
    engine->registerSymbol("BTCUSDT",100000); 

    EngineRouter::instance().bindSymbolToEngine("AAPL", engine);
    EngineRouter::instance().bindSymbolToEngine("BTCUSDT", engine);

    engine->startEngine();

    dispatcher.attachEngine(engine);

    net::TcpServer server(reactor, dispatcher, "0.0.0.0", 9000);
    server.startServer();

    dispatcher.setSender([&](int fd, const std::string& payload) {
        auto* conn = server.getConnection(fd);
        return conn && conn->send(payload);
    });

    LOG_INFO("[Main] Reactor loop started (listening on port 9000)...");
    reactor.runEventLoop();

    dispatcher.stopDispatcher();
    engine->stopEngine();
    LOG_INFO("[Main] OrderBookEngine shutdown.");
    return 0;
}
