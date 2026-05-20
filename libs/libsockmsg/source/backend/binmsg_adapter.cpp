#include "../../include/backend/binmsg_adapter.hpp"
#include "../../include/backend/details/other.hpp"

namespace sockmsg::backend {

int BinMsgAdapter::begin(const char* prefix, const char* type)
{
    m_out_header.pref = details::bmsg::Char64(prefix);
    m_out_header.type = details::bmsg::Char64(type);
    m_out_header.id = 0;
    m_out_header.len = 0;
    m_out_header.flags = 0;
    m_out_len = 0;
    return 0;
}

int BinMsgAdapter::writeBytes(const void* src, size_t size)
{
    if (static_cast<size_t>(m_out_len) + size > kMaxPayload) {
        return 1;
    }

    details::memcpy(m_out_payload + m_out_len, src, size);
    m_out_len = static_cast<uint16_t>(m_out_len + size);
    return 0;
}

int BinMsgAdapter::writeI8(int64_t value)
{
    int8_t narrowed = static_cast<int8_t>(value);
    return writeBytes(&narrowed, sizeof(narrowed));
}

int BinMsgAdapter::writeI16(int64_t value)
{
    int16_t narrowed = static_cast<int16_t>(value);
    return writeBytes(&narrowed, sizeof(narrowed));
}

int BinMsgAdapter::writeI32(int64_t value)
{
    int32_t narrowed = static_cast<int32_t>(value);
    return writeBytes(&narrowed, sizeof(narrowed));
}

int BinMsgAdapter::writeI64(int64_t value)
{
    return writeBytes(&value, sizeof(value));
}

int BinMsgAdapter::writeId(int64_t value)
{
    uint32_t narrowed = static_cast<uint32_t>(value);
    return writeBytes(&narrowed, sizeof(narrowed));
}

int BinMsgAdapter::writeBool(int64_t value)
{
    uint8_t narrowed = value != 0;
    return writeBytes(&narrowed, sizeof(narrowed));
}

int BinMsgAdapter::writeString(const char* value)
{
    size_t len = details::strlen(value);
    if (len > UINT16_MAX) {
        return 1;
    }

    uint16_t wire_len = static_cast<uint16_t>(len);
    if (writeBytes(&wire_len, sizeof(wire_len)) != 0) {
        return 1;
    }
    return writeBytes(value, len);
}

int BinMsgAdapter::writeChar64(const char* value)
{
    details::bmsg::Char64 char64(value);
    return writeBytes(&char64, sizeof(char64));
}

int BinMsgAdapter::send(SocketAdapter* socket)
{
    m_out_header.len = m_out_len;

    int rc = socket->write_exact(&m_out_header, sizeof(m_out_header));
    if (rc != 0) {
        return rc;
    }
    if (m_out_header.len == 0) {
        return 0;
    }
    return socket->write_exact(m_out_payload, m_out_header.len);
}

int BinMsgAdapter::readNext(SocketAdapter* socket)
{
    int rc = socket->read_exact(m_in_message, kHeaderSize);
    if (rc != 0) {
        m_has_message = 0;
        if (rc == 2) {
            socket->set_dead();
        }
        return rc;
    }

    details::bmsg::RawMessage header_only(reinterpret_cast<const char*>(m_in_message), kHeaderSize);
    const details::bmsg::Header* header = header_only.header();
    if (!header) {
        m_has_message = 0;
        return -1;
    }

    m_in_header = *header;
    if (m_in_header.len > kMaxPayload) {
        m_has_message = 0;
        socket->set_dead();
        return -1;
    }

    if (m_in_header.len > 0) {
        rc = socket->read_exact(m_in_message + kHeaderSize, m_in_header.len);
        if (rc != 0) {
            m_has_message = 0;
            if (rc == 2) {
                socket->set_dead();
            }
            return rc;
        }
    }

    details::bmsg::RawMessage full(reinterpret_cast<const char*>(m_in_message),
                                   kHeaderSize + m_in_header.len);
    if (!full.is_correct()) {
        m_has_message = 0;
        return -1;
    }

    m_has_message = 1;
    return 0;
}

int64_t BinMsgAdapter::prefix() const
{
    return m_has_message ? static_cast<int64_t>(m_in_header.pref.as_u64) : 0;
}

int64_t BinMsgAdapter::type() const
{
    return m_has_message ? static_cast<int64_t>(m_in_header.type.as_u64) : 0;
}

int BinMsgAdapter::prefixIs(const char* prefix) const
{
    return m_has_message && m_in_header.pref == prefix;
}

int BinMsgAdapter::typeIs(const char* type) const
{
    return m_has_message && m_in_header.type == type;
}

int BinMsgAdapter::id() const
{
    return m_has_message ? static_cast<int>(m_in_header.id) : 0;
}

int BinMsgAdapter::len() const
{
    return m_has_message ? static_cast<int>(m_in_header.len) : 0;
}

int BinMsgAdapter::flags() const
{
    return m_has_message ? static_cast<int>(m_in_header.flags) : 0;
}

const uint8_t* BinMsgAdapter::payloadAt(int64_t offset, size_t size) const
{
    if (!m_has_message || offset < 0) {
        return nullptr;
    }

    size_t pos = static_cast<size_t>(offset);
    if (pos + size > m_in_header.len) {
        return nullptr;
    }

    return m_in_message + kHeaderSize + pos;
}

int BinMsgAdapter::readI8(int64_t offset) const
{
    const uint8_t* ptr = payloadAt(offset, sizeof(int8_t));
    return ptr ? *reinterpret_cast<const int8_t*>(ptr) : 0;
}

int BinMsgAdapter::readI16(int64_t offset) const
{
    const uint8_t* ptr = payloadAt(offset, sizeof(int16_t));
    return ptr ? *reinterpret_cast<const int16_t*>(ptr) : 0;
}

int BinMsgAdapter::readI32(int64_t offset) const
{
    const uint8_t* ptr = payloadAt(offset, sizeof(int32_t));
    return ptr ? *reinterpret_cast<const int32_t*>(ptr) : 0;
}

int64_t BinMsgAdapter::readI64(int64_t offset) const
{
    const uint8_t* ptr = payloadAt(offset, sizeof(int64_t));
    return ptr ? *reinterpret_cast<const int64_t*>(ptr) : 0;
}

int BinMsgAdapter::readU32(int64_t offset) const
{
    const uint8_t* ptr = payloadAt(offset, sizeof(uint32_t));
    return ptr ? static_cast<int>(*reinterpret_cast<const uint32_t*>(ptr)) : 0;
}

int BinMsgAdapter::readBool(int64_t offset) const
{
    const uint8_t* ptr = payloadAt(offset, sizeof(uint8_t));
    return ptr ? (*ptr != 0) : 0;
}

int BinMsgAdapter::stringEq(int64_t offset, const char* expected) const
{
    const uint8_t* len_ptr = payloadAt(offset, sizeof(uint16_t));
    if (!len_ptr) {
        return 0;
    }

    uint16_t str_len = *reinterpret_cast<const uint16_t*>(len_ptr);
    const uint8_t* str_ptr = payloadAt(offset + sizeof(uint16_t), str_len);
    if (!str_ptr || details::strlen(expected) != str_len) {
        return 0;
    }

    for (uint16_t i = 0; i < str_len; i++) {
        if (str_ptr[i] != static_cast<uint8_t>(expected[i])) {
            return 0;
        }
    }
    return 1;
}

int BinMsgAdapter::stringSize(int64_t offset) const
{
    const uint8_t* len_ptr = payloadAt(offset, sizeof(uint16_t));
    if (!len_ptr) {
        return 0;
    }

    uint16_t str_len = *reinterpret_cast<const uint16_t*>(len_ptr);
    if (!payloadAt(offset + sizeof(uint16_t), str_len)) {
        return 0;
    }

    return static_cast<int>(sizeof(uint16_t) + str_len);
}

int BinMsgAdapter::char64Eq(int64_t offset, const char* expected) const
{
    const uint8_t* ptr = payloadAt(offset, sizeof(details::bmsg::Char64));
    if (!ptr) {
        return 0;
    }

    const details::bmsg::Char64* value = reinterpret_cast<const details::bmsg::Char64*>(ptr);
    return *value == expected;
}

}
