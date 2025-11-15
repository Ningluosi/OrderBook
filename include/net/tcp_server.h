#pragma once
#include <unordered_map>
#include "utils/thread_pool.h"
#include "net/epoll_reactor.h"
#include "net/tcp_connection.h"
#include "utils/socketops.h"
#include "dispatch/dispatcher.h"

namespace net {

class TcpServer {
public:
    explicit TcpServer(EpollReactor& reactor,
                    dispatch::Dispatcher& dispatcher,
                    const std::string& host,
                    uint16_t port,
                    size_t threadCount = std::thread::hardware_concurrency());

    ~TcpServer();

    bool startServer();
    void shutdownServer();

    TcpConnection* getConnection (int fd);
    const TcpConnection* getConnection (int fd) const;

private:
    bool startListening();
    void handleAccept(int listenFd, uint32_t events);
    void handleRead(int connFd, uint32_t events);

private:
    EpollReactor& reactor_;
    utils::ThreadPool threadPool_;
    dispatch::Dispatcher& dispatcher_;

    int listenFd_{-1};
    std::unordered_map<int, TcpConnection> conns_;
};

}
