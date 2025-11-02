#include "net/tcp_server.h"
#include "utils/logger.h"
#include <netinet/in.h>

using namespace utils;

namespace net {

TcpServer::TcpServer(EpollReactor& reactor, const std::string& host, uint16_t port)
    : reactor_(reactor) {
    listenFd_ = createListenSocket(host, port, 128);
    if (listenFd_ < 0) {
        LOG_ERROR("[TcpServer] Failed to create listen socket");
        throw std::runtime_error("listen socket error");
    }
}

bool TcpServer::start() {
    LOG_INFO("[TcpServer] Listening for connections...");
    reactor_.addFd(listenFd_, EPOLLIN, [this](int fd, uint32_t events) {
        handleAccept(fd, events);
    });
    return true;
}

void TcpServer::handleAccept(int listenFd, uint32_t) {
    sockaddr_in client{};
    socklen_t len = sizeof(client);
    int connFd = accept4(listenFd, reinterpret_cast<sockaddr*>(&client), &len, SOCK_NONBLOCK);
    if (connFd < 0) {
        int err = errno;
        if (err == EAGAIN || err == EWOULDBLOCK) {
            return;
        }
        LOG_ERROR("[TcpServer] accept4() failed, errno=" + std::to_string(err) +
                  " (" + std::string(strerror(err)) + ")");
    }

    conns_.emplace(connFd, TcpConnection(connFd));
    reactor_.addFd(connFd, EPOLLIN, [this](int fd, uint32_t events) {
        handleRead(fd, events);
    });

    LOG_INFO("[TcpServer] New connection accepted, fd=" + std::to_string(connFd));
}

void TcpServer::handleRead(int clientFd, uint32_t) {
    auto it = conns_.find(clientFd);
    if (it == conns_.end()) {
        LOG_WARN("[TcpServer] Read event for unknown fd=" + std::to_string(clientFd));
        return;
    }
    auto data = it->second.read();
    if (data.empty()) {
        LOG_INFO("[TcpServer] Connection closed, fd=" + std::to_string(clientFd));
        reactor_.delFd(clientFd);
        conns_.erase(it);
        return;
    }
    if (msgCallback_) msgCallback_(clientFd, data);
}

TcpServer::~TcpServer() {
    if (listenFd_ >= 0) close(listenFd_);
}

}