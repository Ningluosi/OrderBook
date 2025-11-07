#include "utils/socketops.h"
#include "utils/logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>


namespace utils {

bool setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

bool setReuseAddr(int fd) {
    int yes = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == 0;
}

bool setReusePort(int fd) {
    int yes = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) == 0;
}

void ignoreSigpipe() {
    signal(SIGPIPE, SIG_IGN);
}

int createListenSocket(const std::string& host, uint16_t port, int backlog) {
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        LOG_ERROR("[SocketOps] socket() failed: " + std::string(std::strerror(errno)));
        return -1;
    }

    setReuseAddr(fd);
    setReusePort(fd);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (host.empty() || host == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
            LOG_ERROR("[SocketOps] Invalid host address: " + host);
            close(fd);
            return -1;
        }
    }

    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        LOG_ERROR("[SocketOps] bind() failed: " + std::string(std::strerror(errno)));
        close(fd);
        return -1;
    }

    setNonBlocking(fd);
    if (listen(fd, backlog) < 0) {
        LOG_ERROR("[SocketOps] listen() failed: " + std::string(std::strerror(errno)));
        close(fd);
        return -1;
    }

    LOG_INFO("[SocketOps] Listening on " + host + ":" + std::to_string(port));
    return fd;
}

}