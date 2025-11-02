#pragma once
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace net {

using EventCallback = std::function<void(int fd, uint32_t events)>;

class Reactor {
public:
    virtual ~Reactor() = default;
    virtual bool registerEventHandler(int fd, uint32_t events, const EventCallback& cb) = 0;
    virtual bool updateEventMask(int fd, uint32_t events) = 0;
    virtual bool unregisterEventHandler(int fd) = 0;
    virtual void runEventLoop() = 0;
    virtual void stopEventLoop() = 0;

protected:
    std::unordered_map<int, EventCallback> callbacks_;
};

}
