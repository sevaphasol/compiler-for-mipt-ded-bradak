#include "../../include/api/libstd.hpp"

#include "../../include/backend/libstd_backend.hpp"

extern "C" {

void* memcpy_vabi(void* dst, const void* src, size_t size)
{
    return spl::libstd::backend::memcpy(dst, src, size);
}

void* memset_vabi(void* dst, int value, size_t size)
{
    return spl::libstd::backend::memset(dst, value, size);
}

size_t strlen_vabi(const char* str)
{
    return spl::libstd::backend::strlen(str);
}

int strcmp_vabi(const char* lhs, const char* rhs)
{
    return spl::libstd::backend::strcmp(lhs, rhs);
}

int print_num_vabi(int64_t value)
{
    spl::libstd::backend::print_num(value);
    return 0;
}

int print_str_vabi(const char* str)
{
    spl::libstd::backend::print_str(str);
    return 0;
}

int64_t scan_num_vabi(void)
{
    return spl::libstd::backend::scan_num();
}

int equal_vabi(int64_t lhs, int64_t rhs)
{
    return spl::libstd::backend::equal(lhs, rhs);
}

int not_equal_vabi(int64_t lhs, int64_t rhs)
{
    return spl::libstd::backend::not_equal(lhs, rhs);
}

int smaller_vabi(int64_t lhs, int64_t rhs)
{
    return spl::libstd::backend::smaller(lhs, rhs);
}

int bigger_vabi(int64_t lhs, int64_t rhs)
{
    return spl::libstd::backend::bigger(lhs, rhs);
}

int smaller_or_eq_vabi(int64_t lhs, int64_t rhs)
{
    return spl::libstd::backend::smaller_or_eq(lhs, rhs);
}

int bigger_or_eq_vabi(int64_t lhs, int64_t rhs)
{
    return spl::libstd::backend::bigger_or_eq(lhs, rhs);
}

int64_t random_mod_vabi(int64_t max)
{
    return spl::libstd::backend::random_mod(max);
}

int streq_vabi(const char* lhs, const char* rhs)
{
    return spl::libstd::backend::strcmp(lhs, rhs) == 0;
}

}
