/**
 * @file PosixIO.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "utils/PosixIO.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>

#include <cerrno>
#include <cstring>

namespace Utils
{

PosixIO::PosixIO(std::string_view path, int flags, ::mode_t mode)
    : path_{path}
{
    int fd;

    // Always close on executable exit
    if(flags & O_CREAT)
    {
        fd = ::open(path.data(), flags|O_CLOEXEC, mode);
    }
    else
    {
        fd = ::open(path.data(), flags|O_CLOEXEC);
    }

    fd_ = FileDescriptor(fd);
}

PosixIO::PosixIO(int dirfd, std::string_view path, int flags, ::mode_t mode)
    : path_{path}
{
    int fd;

    // Always close on executable exit
    if(flags & O_CREAT)
    {
        fd = ::openat(dirfd, path.data(), flags|O_CLOEXEC, mode);
    }
    else
    {
        fd = ::openat(dirfd, path.data(), flags|O_CLOEXEC);
    }

    fd_ = FileDescriptor(fd);
}

ssize_t PosixIO::pread(void* buf, size_t size)
{
    // Retry on EINTR !
    ssize_t ret;

    do 
    {
        ret = ::pread(fd_.get(), buf, size, 0);
    } 
    while (ret < 0 && errno == EINTR);

    if(ret < 0)
    {
        return static_cast<ssize_t>(-errno);
    }

    return ret;
}

ssize_t PosixIO::write(const void* buf, size_t size)
{
    // Retry on EINTR !
    ssize_t ret;

    do 
    {
        ret = ::write(fd_.get(), buf, size);
    } 
    while (ret < 0 && errno == EINTR);

    if(ret < 0)
    {
        return static_cast<ssize_t>(-errno);
    }

    return ret;
}

int PosixIO::pioCtl(unsigned long req, void* arg)
{
    // Retry on EINTR !
    ssize_t ret;

    do 
    {
        ret = ::ioctl(fd_.get(), req, arg);
    } 
    while (ret < 0 && errno == EINTR);

    if(ret < 0)
    {
        return static_cast<ssize_t>(-errno);
    }

    return ret;
}

int PosixIO::fd() const noexcept
{
    return fd_.get();
}

std::string_view PosixIO::path() const noexcept
{
    return path_;
}

PosixIO::operator bool() const noexcept
{
    return fd_.get() >= 0;
}

}//namespace Utils