#pragma once

#include "../backend/socket_adapter.hpp"
#include "../backend/details/binmsg.hpp"

#include <cstddef>
#include <cstdint>

namespace sockmsg::backend {

class BinMsgAdapter {
	static constexpr size_t kMaxPayload = 4096;
	static constexpr size_t kHeaderSize = sizeof(details::bmsg::Header);
	static constexpr size_t kMaxMessage = kHeaderSize + kMaxPayload;

    uint8_t m_in_message[kMaxMessage]{};
    uint8_t m_out_payload[kMaxPayload]{};
    details::bmsg::Header m_out_header{};
    details::bmsg::Header m_in_header{};
    uint16_t m_out_len{};
    int m_has_message{};

  public:
    int begin(const char* prefix, const char* type);

    int writeBytes(const void* src, size_t size);
    int writeI8(int64_t value);
    int writeI16(int64_t value);
    int writeI32(int64_t value);
    int writeI64(int64_t value);
    int writeId(int64_t value);
    int writeBool(int64_t value);
    int writeString(const char* value);
    int writeChar64(const char* value);

    int send(SocketAdapter* socket);
    int readNext(SocketAdapter* socket);
    
	int64_t prefix() const;
    int64_t type() const;
    
	int prefixIs(const char* prefix) const;
    int typeIs(const char* type) const;
    
	int id() const;
    int len() const;
    int flags() const;
    const uint8_t* payloadAt(int64_t offset, size_t size) const;
    
	int readI8(int64_t offset) const;
    int readI16(int64_t offset) const;
    int readI32(int64_t offset) const;
    int64_t readI64(int64_t offset) const;
	int readU32(int64_t offset) const;
    int readBool(int64_t offset) const;
    
	int stringEq(int64_t offset, const char* expected) const;
    int stringSize(int64_t offset) const;
    int char64Eq(int64_t offset, const char* expected) const;
};

}
