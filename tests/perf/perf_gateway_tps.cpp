#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "net/epoll_reactor.h"
#include "net/tcp_server.h"
#include "dispatch/dispatcher.h"
#include "engine/matching_engine.h"
#include "engine/engine_router.h"
#include "utils/logger.h"

using namespace std::chrono;
using namespace net;
using namespace dispatch;
using namespace engine;
using namespace utils;

static std::atomic<uint64_t> gAcceptCount{0};
static std::atomic<uint64_t> gReadCount{0};

class TpsTcpServer : public TcpServer {
public:
    TpsTcpServer(EpollReactor& r, Dispatcher& d,
                 const std::string& ip, int port)
        : TcpServer(r, d, ip, port) {}

protected:
    void handleAccept(int listenFd, uint32_t events) override {
        gAcceptCount.fetch_add(1, std::memory_order_relaxed);
        TcpServer::handleAccept(listenFd, events);
    }

    void handleRead(int connFd, uint32_t events) override {
        gReadCount.fetch_add(1, std::memory_order_relaxed);
        TcpServer::handleRead(connFd, events);
    }
};

void clientStormWorker(int port, std::atomic<bool>& running, int clientId) {
    const char* msg =
        "{\"type\":\"NEW_ORDER\",\"symbol\":\"AAPL\",\"side\":\"BUY\","
        "\"price\":100.01,\"qty\":100}\n";

    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd < 0) {
        std::cerr << "[client " << clientId << "] socket fail\n";
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    int ret = ::connect(fd, (sockaddr*)&addr, sizeof(addr));
    if (ret != 0 && errno != EINPROGRESS) {
        std::cerr << "[client " << clientId << "] connect fail\n";
        ::close(fd);
        return;
    }

    while (running.load()) {
        socklen_t len = sizeof(ret);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &ret, &len) == 0 && ret == 0) {
            break;
        }
    }


    while (running.load(std::memory_order_acquire)) {
        ::send(fd, msg, std::strlen(msg), MSG_NOSIGNAL);
    }

    ::close(fd);
}

int main() {
    const int PORT       = 9000;
    const int TEST_SEC   = 5;
    const int CLIENT_NUM = 8;

    std::cout << "=== Gateway TPS Benchmark Starting ===" << std::endl;

    EpollReactor reactor;

    Dispatcher dispatcher(1024);
    dispatcher.startDispatcher();

    auto* engine = new MatchingEngine();

    engine->registerSymbol("AAPL",    100000);
    engine->registerSymbol("TESLA", 100000);

    EngineRouter::instance().bindSymbolToEngine("AAPL",    engine);
    EngineRouter::instance().bindSymbolToEngine("TESLA", engine);

    engine->startEngine();
    dispatcher.attachEngine(engine);

    TpsTcpServer server(reactor, dispatcher, "0.0.0.0", PORT);
    server.startServer();

    dispatcher.setSender([&](int fd, const std::string& payload){
        return true;
    });

    std::thread reactorThread([&]{
        reactor.runEventLoop();
    });

    std::atomic<bool> running{true};
    std::vector<std::thread> clients;
    clients.reserve(CLIENT_NUM);
    for (int i = 0; i < CLIENT_NUM; ++i) {
        clients.emplace_back(clientStormWorker, PORT, std::ref(running), i + 1);
    }

    auto t0 = steady_clock::now();
    std::this_thread::sleep_for(std::chrono::seconds(TEST_SEC));
    running.store(false, std::memory_order_release);

    for (auto& t : clients) t.join();

    auto t1 = steady_clock::now();
    double sec = std::max(1e-6, duration<double>(t1 - t0).count());

    uint64_t totalA = gAcceptCount.load(std::memory_order_relaxed);
    uint64_t totalR = gReadCount.load(std::memory_order_relaxed);

    std::cout << "\n===== Gateway TPS Final Results =====\n";
    std::cout << "[Total Accept] = " << totalA << "\n";
    std::cout << "[Total Read  ] = " << totalR << "\n";
    std::cout << "[Accept TPS  ] = " << (totalA / sec) << "\n";
    std::cout << "[Read   TPS  ] = " << (totalR / sec) << "\n";
    std::cout << "[Benchmark] Gateway TPS Benchmark Finished.\n";
    
    ::_exit(0);
}
