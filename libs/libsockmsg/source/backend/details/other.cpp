#include "../../../include/backend/details/other.hpp"

#include <cstdint>

namespace sockmsg::backend::details {

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

size_t strlen(const char* str)
{
    size_t len = 0;
    while (str && str[len] != '\0') {
        len++;
    }
    return len;
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

}
