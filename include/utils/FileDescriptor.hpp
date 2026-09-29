/**
 * @file FileDescriptor.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once
#include <string_view>

namespace Utils
{

class FileDescriptor
{

public:
    FileDescriptor() = default;
    explicit FileDescriptor(int fd) noexcept;

    ~FileDescriptor();

    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    FileDescriptor(FileDescriptor&& other) noexcept;
    FileDescriptor& operator=(FileDescriptor&& other) noexcept;
    
    explicit operator bool() const noexcept;

    [[nodiscard]] int get() const noexcept;
    void reset(int newFd = -1);

    int release() noexcept;

private:
    int fd_{-1};
};

} // namespace Utils