#pragma once

#include <cstdint>

extern "C" {

int socket_connect_raw_vabi(const char* host, const char* port);
int socket_alive_vabi(void);
int socket_set_dead_vabi(void);
int socket_read_next_vabi(void);
int socket_close_vabi(void);
int socket_fd_vabi(void);

int binmsg_begin_vabi(const char* prefix, const char* type);
int binmsg_write_i8_vabi(int64_t value);
int binmsg_write_i16_vabi(int64_t value);
int binmsg_write_i32_vabi(int64_t value);
int binmsg_write_i64_vabi(int64_t value);
int binmsg_write_id_vabi(int64_t value);
int binmsg_write_bool_vabi(int64_t value);
int binmsg_write_string_vabi(const char* value);
int binmsg_write_char64_vabi(const char* value);
int binmsg_send_vabi(void);

int64_t binmsg_prefix_vabi(void);
int64_t binmsg_type_vabi(void);
int binmsg_prefix_is_vabi(const char* prefix);
int binmsg_type_is_vabi(const char* type);
int binmsg_id_vabi(void);
int binmsg_len_vabi(void);
int binmsg_flags_vabi(void);
int binmsg_read_i8_vabi(int64_t offset);
int binmsg_read_i16_vabi(int64_t offset);
int binmsg_read_i32_vabi(int64_t offset);
int64_t binmsg_read_i64_vabi(int64_t offset);
int binmsg_read_u32_vabi(int64_t offset);
int binmsg_read_bool_vabi(int64_t offset);
int binmsg_string_eq_vabi(int64_t offset, const char* expected);
int binmsg_string_size_vabi(int64_t offset);
int binmsg_char64_eq_vabi(int64_t offset, const char* expected);

}
