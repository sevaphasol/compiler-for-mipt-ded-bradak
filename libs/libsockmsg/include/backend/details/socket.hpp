#pragma once

#include <cstddef>
#include <cstdint>

namespace sockmsg::backend::details::socket {

constexpr int kIPv4Domain = 2;
constexpr int kStreamSocketType = 1;
constexpr int kProtocolTCP = 0;
constexpr int kIoTimeoutMs = 100;

struct SockAddrIn {
    uint16_t sin_family;
    uint16_t sin_port;
    uint32_t sin_addr;
    uint8_t sin_zero[8];
};

struct FdSet {
    unsigned long fds_bits[1024 / (8 * sizeof(long))];
};

struct TimeVal {
    long tv_sec;
    long tv_usec;
};

void fd_zero(FdSet* set);
void fd_set(int fd, FdSet* set);

uint16_t host_to_be16(uint16_t value);
uint32_t parse_ipv4(const char* host);
uint16_t parse_port(const char* port);

long socket_create_tcp_ipv4();
long socket_connect(int fd, const void* addr, size_t addr_len);
long socket_send(int fd, const void* buf, size_t len, int flags);
long socket_recv(int fd, void* buf, size_t len, int flags);
long socket_select(int nfds, void* readfds, void* writefds, void* exceptfds, void* timeout);
long socket_close(int fd);

}
