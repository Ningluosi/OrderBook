#include "engine/epoll_reactor.h"
#include "utils/logger.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <stdexcept>

using namespace utils;

namespace net {

EpollReactor::EpollReactor(int maxEvents, int timeoutMs)
    : timeoutMs_(timeoutMs), events_(maxEvents) {
    epfd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epfd_ < 0) {
        LOG_ERROR("[EpollReactor] epoll_create1 failed: " + std::string(std::strerror(errno)));
        throw std::runtime_error("Failed to create epoll instance");
    }
}

EpollReactor::~EpollReactor() {
    if (epfd_ >= 0) close(epfd_);
}

bool EpollReactor::registerEventHandler(int fd, uint32_t events, const EventCallback& cb) {
    struct epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        LOG_ERROR("[EpollReactor] epoll_ctl ADD failed: " + std::string(std::strerror(errno)));
        return false;
    }
    callbacks_[fd] = cb;
    return true;
}

bool EpollReactor::updateEventMask(int fd, uint32_t events) {
    struct epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    return epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}

bool EpollReactor::unregisterEventHandler(int fd) {
    if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        LOG_ERROR("[EpollReactor] epoll_ctl DEL failed: " + std::string(std::strerror(errno)));
        return false;
    }
    callbacks_.erase(fd);
    return true;
}

void EpollReactor::runEventLoop() {
    reactorRunning_ = true;
    while (reactorRunning_) {
        int n = epoll_wait(epfd_, events_.data(), static_cast<int>(events_.size()), timeoutMs_);

        if (n == (int)events_.size()) events_.resize(events_.size() * 2);

        if (n < 0) {
            if (errno == EINTR) continue;
            LOG_ERROR("[EpollReactor] epoll_wait failed: " + std::string(std::strerror(errno)));
            break;
        }
        for (int i = 0; i < n; ++i) {
            int fd = events_[i].data.fd;
            auto it = callbacks_.find(fd);
            if (it != callbacks_.end()) {
                try {
                    it->second(fd, events_[i].events);
                } catch (const std::exception& ex) {
                    LOG_ERROR("[EpollReactor] Exception in callback for fd " +
                              std::to_string(fd) + ": " + ex.what());
                } catch (...) {
                    LOG_ERROR("[EpollReactor] Unknown exception in callback for fd " +
                              std::to_string(fd));
                }
            }
        }
    }
}

void EpollReactor::stopEventLoop() { reactorRunning_ = false; }

}