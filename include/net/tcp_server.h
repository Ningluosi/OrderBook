#pragma once
#include "net/epoll_reactor.h"
#include "net/tcp_connection.h"
#include "utils/socketops.h"
#include <unordered_map>
#include <functional>

namespace net {

class TcpServer {
public:
    using MessageCallback = std::function<void(int fd, const std::string&)>;

    TcpServer(EpollReactor& reactor, const std::string& host, uint16_t port);
    ~TcpServer();

    bool startListening();
    void setMessageCallback(const MessageCallback& cb) { msgCallback_ = cb; }

private:
    void handleAccept(int listenFd, uint32_t events);
    void handleRead(int clientFd, uint32_t events);

private:
    EpollReactor& reactor_;
    int listenFd_{-1};
    std::unordered_map<int, TcpConnection> conns_;
    MessageCallback msgCallback_;
};

}
