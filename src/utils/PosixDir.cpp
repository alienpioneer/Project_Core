/**
 * @file PosixDir.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */
#include "utils/PosixDir.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>

namespace Utils
{

PosixDir ::PosixDir(const std::string& path)
    : path_{path}
{
    int fd = ::open(path.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    fd_ = FileDescriptor(fd);
}

int PosixDir::fd() const noexcept
{
     return fd_.get();
}

std::string_view PosixDir::path() const noexcept
{
    return path_;
}

PosixDir::operator bool() const noexcept
{
    return fd_.get() >= 0;
}

std::string PosixDir::joinPath(std::string_view base, std::string_view name) noexcept
{
    std::string result(base);

    if (!result.empty() && result.back() != '/')
        result += '/';

    if (!name.empty() && name.front() == '/')
        result += name.substr(1);
    else
        result += name;

    return result;
}

}// namespace Utils