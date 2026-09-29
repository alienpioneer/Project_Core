/**
 * @file FileDescriptor.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "utils/FileDescriptor.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <utility>

namespace Utils
{

FileDescriptor::FileDescriptor(int fd) noexcept
    : fd_{fd}
{}

FileDescriptor::~FileDescriptor()
{
    reset();
}

void FileDescriptor::reset(int newFd)
{
    if (fd_ >= 0)
    {
        ::close(fd_);
    }
    fd_= newFd;
}

FileDescriptor::FileDescriptor(FileDescriptor&& other) noexcept
{
    fd_ = std::exchange(other.fd_, -1);
}

FileDescriptor& FileDescriptor::operator=(FileDescriptor&& other) noexcept
{
    if (this != &other)
    {
        reset();
        fd_ = other.release();
    }

    return *this;
}

int FileDescriptor::get() const noexcept
{
    return fd_;
}

FileDescriptor::operator bool() const noexcept
{
    return fd_ >= 0;
}

int FileDescriptor::release() noexcept
{
    return std::exchange(fd_, -1);
}
    
} // namespace Utils