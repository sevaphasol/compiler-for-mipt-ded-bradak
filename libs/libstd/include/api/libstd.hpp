#pragma once

#include <cstdint>
#include <cstddef>

extern "C" {

void* memcpy_vabi(void* dst, const void* src, size_t size);
void* memset_vabi(void* dst, int value, size_t size);
size_t strlen_vabi(const char* str);
int strcmp_vabi(const char* lhs, const char* rhs);
int print_num_vabi(int64_t value);
int print_str_vabi(const char* str);
int64_t scan_num_vabi(void);

int equal_vabi(int64_t lhs, int64_t rhs);
int not_equal_vabi(int64_t lhs, int64_t rhs);
int smaller_vabi(int64_t lhs, int64_t rhs);
int bigger_vabi(int64_t lhs, int64_t rhs);
int smaller_or_eq_vabi(int64_t lhs, int64_t rhs);
int bigger_or_eq_vabi(int64_t lhs, int64_t rhs);
int64_t random_mod_vabi(int64_t max);

}
	