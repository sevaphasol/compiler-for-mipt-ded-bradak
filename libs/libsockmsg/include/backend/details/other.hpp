#pragma once

#include <cstddef>

namespace sockmsg::backend::details {

int strcmp(const char* lhs, const char* rhs);
size_t strlen(const char* str);
void* memcpy(void* dst, const void* src, size_t size);

}
