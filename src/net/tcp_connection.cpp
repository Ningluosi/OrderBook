#include "net/tcp_connection.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

namespace net {

std::string TcpConnection::read() {
    char buf[4096];
    ssize_t n = ::recv(fd_, buf, sizeof(buf), 0);
    if (n <= 0) return {};
    return std::string(buf, n);
}

bool TcpConnection::send(const std::string& data) {
    ssize_t n = ::send(fd_, data.data(), data.size(), 0);
    return n == (ssize_t)data.size();
}

}
