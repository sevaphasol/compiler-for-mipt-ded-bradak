#include "../../../include/backend/details/socket.hpp"
#include "../../../include/backend/details/other.hpp"

namespace sockmsg::backend::details {

namespace {

constexpr long kSyscallClose = 3;
constexpr long kSyscallSelect = 23;
constexpr long kSyscallSocketCreate = 41;
constexpr long kSyscallSocketConnect = 42;
constexpr long kSyscallSocketSend = 44;
constexpr long kSyscallSocketRecv = 45;

}

using namespace socket;

void fd_zero(FdSet* set)
{
    for (size_t i = 0; i < sizeof(set->fds_bits) / sizeof(set->fds_bits[0]); i++) {
        set->fds_bits[i] = 0;
    }
}

void fd_set(int fd, FdSet* set)
{
    set->fds_bits[fd / (8 * static_cast<int>(sizeof(long)))] |=
        1UL << (fd % (8 * static_cast<int>(sizeof(long))));
}

uint16_t host_to_be16(uint16_t value)
{
    return static_cast<uint16_t>((value >> 8) | (value << 8));
}

uint32_t parse_ipv4(const char* host)
{
    if (details::strcmp(host, "localhost") == 0) {
        return 127u | (0u << 8) | (0u << 16) | (1u << 24);
    }

    uint32_t ip = 0;
    int shift = 0;
    const char* cur = host;

    for (int i = 0; i < 4; i++) {
        uint32_t octet = 0;
        while (*cur >= '0' && *cur <= '9') {
            octet = octet * 10 + static_cast<uint32_t>(*cur - '0');
            cur++;
        }
        ip |= (octet & 0xffu) << shift;
        shift += 8;
        if (*cur == '.') {
            cur++;
        }
    }

    return ip;
}

uint16_t parse_port(const char* port)
{
    uint16_t value = 0;
    for (const char* cur = port; cur && *cur >= '0' && *cur <= '9'; cur++) {
        value = static_cast<uint16_t>(value * 10 + static_cast<uint16_t>(*cur - '0'));
    }
    return value;
}

long socket_create_tcp_ipv4()
{
    long ret = 0;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(kSyscallSocketCreate), "D"(kIPv4Domain), "S"(kStreamSocketType), "d"(kProtocolTCP)
        : "rcx", "r11", "memory");
    return ret;
}

long socket_connect(int fd, const void* addr, size_t addr_len)
{
    long ret = 0;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(kSyscallSocketConnect), "D"(fd), "S"(addr), "d"(addr_len)
        : "rcx", "r11", "memory");
    return ret;
}

long socket_send(int fd, const void* buf, size_t len, int flags)
{
    long ret = 0;
    register long r10_arg asm("r10") = flags;
    register long r8_arg asm("r8") = 0;
    register long r9_arg asm("r9") = 0;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(kSyscallSocketSend), "D"(fd), "S"(buf), "d"(len),
          "r"(r10_arg), "r"(r8_arg), "r"(r9_arg)
        : "rcx", "r11", "memory");
    return ret;
}

long socket_recv(int fd, void* buf, size_t len, int flags)
{
    long ret = 0;
    register long r10_arg asm("r10") = flags;
    register long r8_arg asm("r8") = 0;
    register long r9_arg asm("r9") = 0;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(kSyscallSocketRecv), "D"(fd), "S"(buf), "d"(len),
          "r"(r10_arg), "r"(r8_arg), "r"(r9_arg)
        : "rcx", "r11", "memory");
    return ret;
}

long socket_select(int nfds, void* readfds, void* writefds, void* exceptfds, void* timeout)
{
    long ret = 0;
    register long r10_arg asm("r10") = reinterpret_cast<long>(exceptfds);
    register long r8_arg asm("r8") = reinterpret_cast<long>(timeout);
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(kSyscallSelect), "D"(nfds), "S"(readfds), "d"(writefds),
          "r"(r10_arg), "r"(r8_arg)
        : "rcx", "r11", "memory");
    return ret;
}

long socket_close(int fd)
{
    long ret = 0;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(kSyscallClose), "D"(fd)
        : "rcx", "r11", "memory");
    return ret;
}

}
