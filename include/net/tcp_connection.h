#pragma once
#include <string>
#include <unistd.h>

namespace net {

class TcpConnection {
public:
    explicit TcpConnection(int fd) : fd_(fd) {}
    ~TcpConnection() { if (fd_ >= 0) ::close(fd_); }

    int socketFd() const { return fd_; }
    std::string read();
    bool send(const std::string& data);

private:
    int fd_;
};

}
