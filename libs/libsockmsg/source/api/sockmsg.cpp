#include "../../include/api/sockmsg.hpp"
#include "../../include/backend/binmsg_adapter.hpp"
#include "../../include/backend/socket_adapter.hpp"

namespace sockmsg::api::details {

static backend::SocketAdapter g_socket;
static backend::BinMsgAdapter g_binmsg;

}

extern "C" {

using namespace sockmsg::api::details;

int socket_connect_raw_vabi(const char* host, const char* port)
{
    return g_socket.connect(host, port);
}

int socket_alive_vabi(void)
{
    return g_socket.alive();
}

int socket_set_dead_vabi(void)
{
    return g_socket.set_dead();
}

int socket_read_next_vabi(void)
{
    return g_binmsg.readNext(&g_socket);
}

int socket_close_vabi(void)
{
    return g_socket.close();
}

int socket_fd_vabi(void)
{
    return g_socket.fd();
}

int binmsg_begin_vabi(const char* prefix, const char* type)
{
    return g_binmsg.begin(prefix, type);
}

int binmsg_write_i8_vabi(int64_t value)
{
    return g_binmsg.writeI8(value);
}

int binmsg_write_i16_vabi(int64_t value)
{
    return g_binmsg.writeI16(value);
}

int binmsg_write_i32_vabi(int64_t value)
{
    return g_binmsg.writeI32(value);
}

int binmsg_write_i64_vabi(int64_t value)
{
    return g_binmsg.writeI64(value);
}

int binmsg_write_id_vabi(int64_t value)
{
    return g_binmsg.writeId(value);
}

int binmsg_write_bool_vabi(int64_t value)
{
    return g_binmsg.writeBool(value);
}

int binmsg_write_string_vabi(const char* value)
{
    return g_binmsg.writeString(value);
}

int binmsg_write_char64_vabi(const char* value)
{
    return g_binmsg.writeChar64(value);
}

int binmsg_send_vabi(void)
{
    return g_binmsg.send(&g_socket);
}

int64_t binmsg_prefix_vabi(void)
{
    return g_binmsg.prefix();
}

int64_t binmsg_type_vabi(void)
{
    return g_binmsg.type();
}

int binmsg_prefix_is_vabi(const char* prefix)
{
    return g_binmsg.prefixIs(prefix);
}

int binmsg_type_is_vabi(const char* type)
{
    return g_binmsg.typeIs(type);
}

int binmsg_id_vabi(void)
{
    return g_binmsg.id();
}

int binmsg_len_vabi(void)
{
    return g_binmsg.len();
}

int binmsg_flags_vabi(void)
{
    return g_binmsg.flags();
}

int binmsg_read_i8_vabi(int64_t offset)
{
    return g_binmsg.readI8(offset);
}

int binmsg_read_i16_vabi(int64_t offset)
{
    return g_binmsg.readI16(offset);
}

int binmsg_read_i32_vabi(int64_t offset)
{
    return g_binmsg.readI32(offset);
}

int64_t binmsg_read_i64_vabi(int64_t offset)
{
    return g_binmsg.readI64(offset);
}

int binmsg_read_u32_vabi(int64_t offset)
{
    return g_binmsg.readU32(offset);
}

int binmsg_read_bool_vabi(int64_t offset)
{
    return g_binmsg.readBool(offset);
}

int binmsg_string_eq_vabi(int64_t offset, const char* expected)
{
    return g_binmsg.stringEq(offset, expected);
}

int binmsg_string_size_vabi(int64_t offset)
{
    return g_binmsg.stringSize(offset);
}

int binmsg_char64_eq_vabi(int64_t offset, const char* expected)
{
    return g_binmsg.char64Eq(offset, expected);
}

}
