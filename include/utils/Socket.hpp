/**
 * @file Socket.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once

#include <unistd.h>
#include <sys/socket.h>

#include "utils/FileDescriptor.hpp"

/**
 * @page SocketPage Socket - RAII wrapper for a POSIX socket file descriptor
 *
 * @section overview Overview
 *
 * The Socket class provides a minimal, generic abstraction over POSIX sockets.
 * It is designed to:
 * - Centralize file descriptor management
 * - Provide a consistent error handling model
 * - Avoid protocol-specific coupling
 *
 * The abstraction is intentionally thin and does not attempt to wrap
 * the entire socket API.
 *
 * @section design Design Principles
 *
 * - RAII ownership of file descriptor
 * - No hidden behavior
 * - No protocol assumptions
 * - Explicit configuration
 *
 * @section error_handling Error Handling
 *
 * All methods follow a unified convention:
 *
 * - `0`   → success
 * - `> 0` → error (errno value)
 *
 * This avoids exceptions and keeps compatibility with low-level systems code.
 *
 * @section responsibilities Responsibilities
 *
 * The Socket class handles:
 * - socket()
 * - bind()
 * - close()
 * - fcntl() blocking flags
 *
 * It does NOT handle:
 * - protocol-specific options (e.g. CAN_RAW, TCP tuning)
 * - address construction
 * - higher-level I/O semantics
 *
 * @section layering Layering Example
 *
 * @code
 * Socket         → generic FD + syscalls
 * CanSocket      → CAN-specific setup
 * NetlinkSocket  → Netlink-specific setup
 * @endcode
 *
 * @section example Example
 *
 * @code
 * Utils::Socket sock;
 *
 * if (sock.open(AF_INET, SOCK_STREAM, 0) != 0)
 *     return;
 *
 * sock.setReuseAddr(true);
 * sock.setNonBlocking(true);
 *
 * sockaddr_in addr{};
 * addr.sin_family = AF_INET;
 * addr.sin_port = htons(8080);
 * addr.sin_addr.s_addr = INADDR_ANY;
 *
 * if (sock.bind(reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
 *     return;
 * @endcode
 *
 * 
 * @section reuse Reuse Address
 * 
 * Allows a socket to bind to an address/port that is still considered “in use” by the kernel.
 * 
 * Typical cases:
 * - Restarting a server quickly
 * - Previous socket is in TIME_WAIT
 * - Without reuse → bind() fails (EADDRINUSE)
 * - With reuse → bind succeeds
 * - Multiple binds (UDP / specific cases) - Allows multiple sockets to bind same address/port (rules depend on OS and protocol)
 * 
 * When to use
 * - TCP servers (recommended)
 * - UDP listeners (common)
 */

namespace Utils
{

/**
 * @class Socket
 * @brief RAII wrapper for a POSIX socket file descriptor.
 *
 * This class encapsulates a socket file descriptor and provides
 * a minimal set of operations required to manage it safely.
 *
 * The class is intentionally lightweight and does not impose
 * protocol-specific behavior.
 *
 * Example usage:
 * @code
 * Utils::Socket sock;
 *
 * if (sock.open(AF_INET, SOCK_STREAM, 0) != 0)
 *     return;
 *
 * sock.setReuseAddr(true);
 * sock.setNonBlocking(true);
 *
 * sockaddr_in addr{};
 * addr.sin_family = AF_INET;
 * addr.sin_port = htons(8080);
 * addr.sin_addr.s_addr = INADDR_ANY;
 *
 * sock.bind(reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
 * @endcode
 */
class Socket
{
public:
    Socket() = default;

    /**
     * @brief Open a socket.
     *
     * If a socket is already open, it is automatically closed.
     *
     * @param domain   Communication domain (e.g. AF_INET, AF_CAN)
     * @param type     Socket type (e.g. SOCK_STREAM, SOCK_RAW)
     * @param protocol Protocol (usually 0 or domain-specific)
     *
     * @return 0 on success, errno on failure
     */
    ssize_t open(int domain, int type, int protocol) noexcept;

    /**
     * @brief Close the socket.
     *
     * Safe to call multiple times.
     *
     * @return Always 0
     */
    ssize_t close() noexcept;

    /**
     * @brief Bind the socket to an address.
     *
     * @param addr Pointer to sockaddr structure
     * @param len  Size of the address structure
     *
     * @return 0 on success, errno on failure
     */
    ssize_t bind(const struct ::sockaddr* addr, ::socklen_t len) noexcept;

    /**
     * @brief Check if the socket is valid.
     *
     * @return true if a valid file descriptor is held
     */
    bool valid() const noexcept;

    /**
     * @brief Get the raw file descriptor.
     *
     * @return file descriptor or -1 if invalid
     */
    int  get() const noexcept;

    /**
     * @brief Enable or disable non-blocking mode.
     *
     * @param enable true to enable, false to disable
     *
     * @return 0 on success, errno on failure
     */
    ssize_t setNonBlocking(bool enable) noexcept;

    /**
     * @brief Enable or disable address reuse.
     *
     * Wraps SO_REUSEADDR socket option.
     *
     * @param enable true to enable, false to disable
     *
     * @return 0 on success, errno on failure
     */
    ssize_t setReuseAddr(bool enable) noexcept;

private:
    Utils::FileDescriptor m_socketFd; /**< Owned socket file descriptor */
};
    
}// namespace Utils