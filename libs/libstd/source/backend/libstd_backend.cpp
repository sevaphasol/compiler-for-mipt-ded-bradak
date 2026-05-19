#include "../../include/backend/libstd_backend.hpp"

namespace spl::libstd::backend {

namespace {

constexpr long kSyscallRead = 0;
constexpr long kSyscallWrite = 1;
constexpr int kStdin = 0;
constexpr int kStdout = 1;
uint64_t g_random_state = 0x9e3779b97f4a7c15ull;

}

void* memcpy(void* dst, const void* src, size_t size)
{
    auto* out = static_cast<uint8_t*>(dst);
    const auto* in = static_cast<const uint8_t*>(src);
    for (size_t i = 0; i < size; i++) {
        out[i] = in[i];
    }
    return dst;
}

void* memset(void* dst, int value, size_t size)
{
    auto* out = static_cast<uint8_t*>(dst);
    for (size_t i = 0; i < size; i++) {
        out[i] = static_cast<uint8_t>(value);
    }
    return dst;
}

size_t strlen(const char* str)
{
    size_t len = 0;
    while (str && str[len] != '\0') {
        len++;
    }
    return len;
}

int strcmp(const char* lhs, const char* rhs)
{
    size_t i = 0;
    while (lhs && rhs && lhs[i] != '\0' && rhs[i] != '\0') {
        if (lhs[i] != rhs[i]) {
            return static_cast<unsigned char>(lhs[i]) - static_cast<unsigned char>(rhs[i]);
        }
        i++;
    }

    if (!lhs && !rhs) return 0;
    if (!lhs) return -static_cast<unsigned char>(rhs[0]);
    if (!rhs) return static_cast<unsigned char>(lhs[0]);
    return static_cast<unsigned char>(lhs[i]) - static_cast<unsigned char>(rhs[i]);
}

long write_fd(int fd, const void* buf, size_t size)
{
    long ret = 0;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(kSyscallWrite), "D"(fd), "S"(buf), "d"(size)
        : "rcx", "r11", "memory");
    return ret;
}

long read_fd(int fd, void* buf, size_t size)
{
    long ret = 0;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(kSyscallRead), "D"(fd), "S"(buf), "d"(size)
        : "rcx", "r11", "memory");
    return ret;
}

void print_str(const char* str)
{
    write_fd(kStdout, str, strlen(str));
}

void print_num(int64_t value)
{
    char buf[32] = {};
    size_t pos = 0;

    if (value == 0) {
        buf[pos++] = '0';
    } else {
        uint64_t num = 0;
        if (value < 0) {
            buf[pos++] = '-';
            num = static_cast<uint64_t>(-value);
        } else {
            num = static_cast<uint64_t>(value);
        }

        char digits[24] = {};
        size_t n_digits = 0;
        while (num > 0) {
            digits[n_digits++] = static_cast<char>('0' + num % 10);
            num /= 10;
        }
        while (n_digits > 0) {
            buf[pos++] = digits[--n_digits];
        }
    }

    buf[pos++] = '\n';
    write_fd(kStdout, buf, pos);
}

int64_t scan_num()
{
    char ch = 0;
    int sign = 1;
    int64_t value = 0;

    do {
        if (read_fd(kStdin, &ch, 1) <= 0) {
            return 0;
        }
    } while (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r');

    if (ch == '-') {
        sign = -1;
        if (read_fd(kStdin, &ch, 1) <= 0) {
            return 0;
        }
    }

    while ('0' <= ch && ch <= '9') {
        value = value * 10 + (ch - '0');
        if (read_fd(kStdin, &ch, 1) <= 0) {
            break;
        }
    }

    return sign * value;
}

int equal(int64_t lhs, int64_t rhs)
{
    return lhs == rhs;
}

int not_equal(int64_t lhs, int64_t rhs)
{
    return lhs != rhs;
}

int smaller(int64_t lhs, int64_t rhs)
{
    return lhs < rhs;
}

int bigger(int64_t lhs, int64_t rhs)
{
    return lhs > rhs;
}

int smaller_or_eq(int64_t lhs, int64_t rhs)
{
    return lhs <= rhs;
}

int bigger_or_eq(int64_t lhs, int64_t rhs)
{
    return lhs >= rhs;
}

int64_t random_mod(int64_t max)
{
    if (max <= 0) {
        return 0;
    }

    g_random_state = g_random_state * 6364136223846793005ull + 1442695040888963407ull;
    return static_cast<int64_t>((g_random_state >> 32) % static_cast<uint64_t>(max));
}

}
