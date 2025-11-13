#include <netinet/in.h>
#include <cstring>
#include "net/tcp_server.h"
#include "utils/logger.h"
#include "utils/message_parser.h"

using namespace utils;

namespace net {

TcpServer::TcpServer(EpollReactor& reactor,
                     utils::ThreadPool& tp,
                     dispatch::Dispatcher& dispatcher,
                     const std::string& host,
                     uint16_t port)
    : reactor_(reactor),
      threadPool_(tp),
      dispatcher_(dispatcher)
{
    listenFd_ = createListenSocket(host, port, 128);
    if (listenFd_ < 0) {
        LOG_ERROR("[TcpServer] Failed to create listen socket");
        throw std::runtime_error("listen socket error");
    }
}

bool TcpServer::startListening() {
    LOG_INFO("[TcpServer] Listening for connections...");
    reactor_.registerEventHandler(listenFd_, EPOLLIN, [this](int fd, uint32_t events) {
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

    conns_.try_emplace(connFd, connFd);

    reactor_.registerEventHandler(connFd, EPOLLIN, [this](int fd, uint32_t events) {
        handleRead(fd, events);
    });

    LOG_INFO("[TcpServer] New connection accepted, fd=" + std::to_string(connFd));
}

void TcpServer::handleRead(int connFd, uint32_t) {
    auto it = conns_.find(connFd);
    if (it == conns_.end()) {
        LOG_WARN("[TcpServer] Read event for unknown fd=" + std::to_string(connFd));
        return;
    }
    auto data = it->second.read();
    if (data.empty()) {
        LOG_INFO("[TcpServer] Connection closed, fd=" + std::to_string(connFd));
        reactor_.unregisterEventHandler(connFd);
        conns_.erase(it);
        return;
    }

    threadPool_.submitTask([this, connFd, raw = std::move(data)]() mutable {
        try {
            auto msg = parseMsg(raw);

            msg.fd = connFd;

            if (!dispatcher_.routeInbound(std::move(msg))) {
                LOG_WARN("[TcpServer] routeInbound failed for fd="
                         + std::to_string(connFd));
            }

        } catch (const std::exception& ex) {
            LOG_ERROR(std::string("[TcpServer] parse or dispatch failed fd=")
                      + std::to_string(connFd) + " ex=" + ex.what());
        }
    });
}

TcpServer::~TcpServer() {
    if (listenFd_ >= 0) close(listenFd_);
}

TcpConnection* TcpServer::getConnection (int fd) {
    auto it = conns_.find(fd);
    if (it == conns_.end()) return nullptr;
    return &(it->second);
}

const TcpConnection* TcpServer::getConnection(int fd) const {
    auto it = conns_.find(fd);
    if (it == conns_.end()) return nullptr;
    return &(it->second);
}

}
