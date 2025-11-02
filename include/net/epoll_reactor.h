#pragma once
#include "net/reactor.h"
#include <vector>
#include <sys/epoll.h>

namespace net {

class EpollReactor final : public Reactor {
public:
    explicit EpollReactor(int maxEvents = 1024, int timeoutMs = -1);
    ~EpollReactor();

    bool registerEventHandler(int fd, uint32_t events, const EventCallback& cb) override;
    bool updateEventMask(int fd, uint32_t events) override;
    bool unregisterEventHandler(int fd) override;
    void runEventLoop() override;
    void stopEventLoop() override;

private:
    int epfd_ = -1;
    int timeoutMs_ = -1;
    bool running_ = false;
    std::vector<struct epoll_event> events_;
};

}