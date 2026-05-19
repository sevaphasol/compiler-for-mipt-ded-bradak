#pragma once

#include <cstddef>
#include <cstdint>

namespace sockmsg::backend::details::bmsg {

class StringView {
    const char* data_ = nullptr;
    size_t size_ = 0;

public:
    constexpr StringView() = default;
    constexpr StringView(const char* data, size_t size) : data_(data), size_(size) {}

    constexpr StringView(const char* str) : data_(str), size_(0) {
        while (str && str[size_] != '\0') {
            size_++;
        }
    }

    constexpr const char* data() const { return data_; }
    constexpr size_t size() const { return size_; }
    constexpr char operator[](size_t idx) const { return data_[idx]; }

    constexpr StringView substr(size_t pos) const {
        if (pos > size_) {
            return {};
        }
        return {data_ + pos, size_ - pos};
    }
};

union Char64 {
    uint64_t as_u64;
    char as_chars[8];

    constexpr Char64() : as_u64(0) {}
    constexpr Char64(uint64_t value) : as_u64(value) {}

    constexpr Char64(StringView str) : as_chars{0, 0, 0, 0, 0, 0, 0, 0} {
        size_t copy_size = str.size() < 8 ? str.size() : 8;
        for (size_t i = 0; i < copy_size; i++) {
            as_chars[i] = str[i];
        }
    }

    constexpr Char64(const char* str) : Char64(StringView(str)) {}

    constexpr size_t size() const {
        size_t size = 0;
        while (size < 8 && as_chars[size] != '\0') {
            size++;
        }
        return size;
    }

    constexpr Char64& operator=(StringView str) {
        *this = Char64(str);
        return *this;
    }

	constexpr bool operator==(const char* other) const {
		return as_u64 == Char64(other).as_u64;
	}

    constexpr operator uint64_t() const { return as_u64; }
    constexpr operator StringView() const { return StringView(as_chars, size()); }
};

constexpr inline bool operator==(Char64 lhs, Char64 rhs) { return lhs.as_u64 == rhs.as_u64; }
constexpr inline bool operator!=(Char64 lhs, Char64 rhs) { return !(lhs == rhs); }

struct Header {
    Char64 pref;
    Char64 type;
    uint32_t id;
    uint16_t len;
    uint16_t flags;
};

class RawMessage {
    StringView data_;

public:
    RawMessage(StringView data) : data_(data) {}
    RawMessage(const char* data, size_t size) : data_(data, size) {}

    StringView data() const { return data_; }
    const Header* header() const;
    StringView body() const;
    const void* body_ptr() const;
    bool is_correct() const;
};

}
