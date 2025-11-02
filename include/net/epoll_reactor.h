#pragma once
#include "net/reactor.h"
#include <vector>
#include <sys/epoll.h>

namespace net {

class EpollReactor final : public Reactor {
public:
    explicit EpollReactor(int maxEvents = 1024, int timeoutMs = -1);
    ~EpollReactor();

    bool addFd(int fd, uint32_t events, const EventCallback& cb) override;
    bool modFd(int fd, uint32_t events) override;
    bool delFd(int fd) override;
    void run() override;
    void stop() override;

private:
    int epfd_ = -1;
    int timeoutMs_ = -1;
    bool running_ = false;
    std::vector<struct epoll_event> events_;
};

}