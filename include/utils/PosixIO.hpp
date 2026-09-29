/**
 * @file PosixIO.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once

#include <unistd.h>

#include <string>

#include "utils/FileDescriptor.hpp"

/*
TODO docs
*/

namespace Utils
{

class PosixIO 
{
public:
    PosixIO() = default;

    explicit PosixIO(std::string_view path, int flags, ::mode_t mode = 0);

    explicit PosixIO(int dirfd, std::string_view path, int flags, ::mode_t mode = 0);

    ssize_t pread(void* buf, size_t size);

    ssize_t write(const void* buf, size_t size);

    int pioCtl(unsigned long req, void* arg = nullptr);

    int fd() const noexcept;

    std::string_view path() const noexcept;

    explicit operator bool() const noexcept;

private:

    Utils::FileDescriptor fd_;
    std::string path_{"None"};
};

}// namespace Utils
