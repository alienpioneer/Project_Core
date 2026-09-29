/**
 * @file PosixDir.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once

#include <unistd.h>

#include <string>

#include "FileDescriptor.hpp"

/*
TODO docs
*/

namespace Utils
{

class PosixDir 
{
public:
    explicit PosixDir (const std::string& path);

    int fd() const noexcept;

    std::string_view path() const noexcept;

    explicit operator bool() const noexcept;

    static std::string joinPath(std::string_view base, std::string_view name) noexcept;

private:

    FileDescriptor fd_;
    std::string path_{"None"};
};

}// namespace Utils
