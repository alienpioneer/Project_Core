/**
 * @file Socket.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "utils/Socket.hpp"

#include <fcntl.h>

#include <cerrno>

namespace Utils
{

ssize_t Socket::open(int domain, int type, int protocol) noexcept
{
    // Auto-close previous fd if already open
    if (m_socketFd)
    {
        m_socketFd.reset();
    }

    m_socketFd = Utils::FileDescriptor(socket(domain, type, protocol));
    
    if (!m_socketFd)
    {
        return errno;
    }

    return 0;
}

ssize_t Socket::bind(const struct ::sockaddr* addr, ::socklen_t len) noexcept
{
    if (!m_socketFd)
    {
        return ENOTRECOVERABLE;
    }   
        
    if (::bind(m_socketFd.get(), addr, len) < 0)
    {
        return errno;
    }

    return 0;
}

bool Socket::valid() const noexcept
{
     return static_cast<bool>(m_socketFd);
}

int Socket::get() const noexcept
{
    return m_socketFd ? m_socketFd.get() : -1;
}

ssize_t Socket::close() noexcept
{
    if (m_socketFd)
    {
        m_socketFd.reset();
    }
        
    return 0;
}

ssize_t Socket::setNonBlocking(bool enable) noexcept
{
    if (!m_socketFd)
    {
        return EBADF;
    }
        
    int flags = fcntl(m_socketFd.get(), F_GETFL, 0);

    if (flags < 0)
    {
        return errno;
    }
        
    if (enable)
    {
        flags |= O_NONBLOCK;
    }
    else
    {
        flags &= ~O_NONBLOCK;
    }
        
    if (fcntl(m_socketFd.get(), F_SETFL, flags) < 0)
    {
        return errno;
    }
        
    return 0;
}

// Allows a socket to bind to an address/port that is still considered “in use” by the kernel
ssize_t Socket::setReuseAddr(bool enable) noexcept
{
    if (!m_socketFd)
    {
        return ENOTRECOVERABLE;
    }
        
    int opt = enable ? 1 : 0;

    if (setsockopt(m_socketFd.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        return errno;
    }

    return 0;
}

}// namespace Utils