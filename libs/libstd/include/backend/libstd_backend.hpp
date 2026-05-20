#pragma once

#include <cstddef>
#include <cstdint>

namespace spl::libstd::backend {

void* memcpy(void* dst, const void* src, size_t size);
void* memset(void* dst, int value, size_t size);
size_t strlen(const char* str);
int strcmp(const char* lhs, const char* rhs);

long write_fd(int fd, const void* buf, size_t size);
long read_fd(int fd, void* buf, size_t size);

void print_num(int64_t value);
void print_str(const char* str);
int64_t scan_num();

int equal(int64_t lhs, int64_t rhs);
int not_equal(int64_t lhs, int64_t rhs);
int smaller(int64_t lhs, int64_t rhs);
int bigger(int64_t lhs, int64_t rhs);
int smaller_or_eq(int64_t lhs, int64_t rhs);
int bigger_or_eq(int64_t lhs, int64_t rhs);
int64_t random_mod(int64_t max);

}
