#pragma once

#include <cstddef>

namespace sockmsg::backend {

class SocketAdapter {
  private:
	int m_fd{};
    int m_alive{};
    int m_inited{};

  public:
    void ensure_init();
    int wait_fd(bool want_write);
    
	int read_exact(void* buf, size_t len);
    int write_exact(const void* buf, size_t len);
    
    int connect(const char* host, const char* port);
	int close();
    
	int alive();
    int set_dead();
    int fd();
};

}
