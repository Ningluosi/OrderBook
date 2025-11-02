#pragma once
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace net {

using EventCallback = std::function<void(int fd, uint32_t events)>;

class Reactor {
public:
    virtual ~Reactor() = default;
    virtual bool addFd(int fd, uint32_t events, const EventCallback& cb) = 0;
    virtual bool modFd(int fd, uint32_t events) = 0;
    virtual bool delFd(int fd) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;

protected:
    std::unordered_map<int, EventCallback> callbacks_;
};

}
