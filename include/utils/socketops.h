#pragma once
#include <string>
#include <cstdint>

namespace utils {

int createListenSocket(const std::string& host, uint16_t port, int backlog = 128);
bool setNonBlocking(int fd);
bool setReuseAddr(int fd);
bool setReusePort(int fd);
void ignoreSigpipe();

}