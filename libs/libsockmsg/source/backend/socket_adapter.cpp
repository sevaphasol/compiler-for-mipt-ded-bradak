#include "../../include/backend/socket_adapter.hpp"
#include "../../include/backend/details/socket.hpp"

#include <cstdint>

namespace sockmsg::backend {

void SocketAdapter::ensure_init()
{
    if (!m_inited) {
        m_fd = -1;
        m_alive = 0;
        m_inited = 1;
    }
}

int SocketAdapter::connect(const char* host, const char* port)
{
    ensure_init();

    int fd = static_cast<int>(details::socket::socket_create_tcp_ipv4());
    if (fd < 0) {
        return 1;
    }

    details::socket::SockAddrIn addr = {};
    addr.sin_family = details::socket::kIPv4Domain;
    addr.sin_port = details::socket::host_to_be16(details::socket::parse_port(port));
    addr.sin_addr = details::socket::parse_ipv4(host);

    if (details::socket::socket_connect(fd, &addr, sizeof(addr)) < 0) {
        details::socket::socket_close(fd);
        return 1;
    }

    m_fd = fd;
    m_alive = 1;
    return 0;
}

int SocketAdapter::wait_fd(bool want_write)
{
    details::socket::FdSet readfds = {};
    details::socket::FdSet writefds = {};
    details::socket::TimeVal timeout = {};

    details::socket::fd_zero(&readfds);
    details::socket::fd_zero(&writefds);
    if (want_write) {
        details::socket::fd_set(m_fd, &writefds);
    } else {
        details::socket::fd_set(m_fd, &readfds);
    }

    timeout.tv_sec = details::socket::kIoTimeoutMs / 1000;
    timeout.tv_usec = (details::socket::kIoTimeoutMs % 1000) * 1000;

    long rc = details::socket::socket_select(m_fd + 1,
                                     want_write ? nullptr : &readfds,
                                     want_write ? &writefds : nullptr,
                                     nullptr,
                                     &timeout);
    if (rc > 0) {
        return 0;
    }
    if (rc == 0) {
        return 1;
    }
    return -1;
}

int SocketAdapter::read_exact(void* buf, size_t len)
{
    uint8_t* out = static_cast<uint8_t*>(buf);
    size_t offset = 0;

    while (offset < len) {
        int wait_rc = wait_fd(false);
        if (wait_rc != 0) {
            return wait_rc;
        }

        long n_read = details::socket::socket_recv(m_fd, out + offset, len - offset, 0);
        if (n_read == 0) {
            return 2;
        }
        if (n_read < 0) {
            return -1;
        }
        offset += static_cast<size_t>(n_read);
    }

    return 0;
}

int SocketAdapter::write_exact(const void* buf, size_t len)
{
    const uint8_t* in = static_cast<const uint8_t*>(buf);
    size_t offset = 0;

    while (offset < len) {
        int wait_rc = wait_fd(true);
        if (wait_rc != 0) {
            return wait_rc;
        }

        long n_written = details::socket::socket_send(m_fd, in + offset, len - offset, 0);
        if (n_written < 0) {
            return -1;
        }
        offset += static_cast<size_t>(n_written);
    }

    return 0;
}

int SocketAdapter::close()
{
    ensure_init();

    int rc = 0;
    if (m_fd >= 0) {
        rc = static_cast<int>(details::socket::socket_close(m_fd));
    }

    m_fd = -1;
    m_alive = 0;
    return rc < 0 ? 1 : 0;
}

int SocketAdapter::alive()
{
    ensure_init();
    return m_alive;
}

int SocketAdapter::set_dead()
{
    ensure_init();
    m_alive = 0;
    return 0;
}

int SocketAdapter::fd()
{
    ensure_init();
    return m_fd;
}

}
