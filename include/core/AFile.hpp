/**
 * @file AFile.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once
#include <filesystem>
#include "AVector.hpp"
#include "Config.hpp"

/**
 * @page AFilePage AFile Class
 * 
 * @brief The base class for file buffering
 *
 * @section Overview
 * Core::AFile represents a single file loaded into memory. It owns a fixed-size buffer
 * and provides access to the file’s path, size, and raw data.
 *
 * The class is move-only to prevent accidental copies of large buffers and ensures
 * RAII semantics: the memory buffer is automatically managed.
 * 
 * Used with @ref FileLoaderPage
 *
 * @section Members
 * - `buffer()` : Access to the internal buffer as `AVector<std::byte, FILE_MAX_SIZE>`.
 * - `data()` : Pointer to the raw buffer data.
 * - `size()` : Number of valid bytes in the buffer.
 * - `path()` : Filesystem path of the loaded file.
 * - `setPath(const std::filesystem::path&)` : Sets the path for the file object.
 *
 * @section Usage
 * ```cpp
 * Core::AFile file;
 * file.setPath("/path/to/file.bin");
 * auto& buf = file.buffer();
 * // fill buf or read data into it
 * std::ifstream f(file.path(), std::ios::binary);
 * f.read(reinterpret_cast<char*>(file.data()), static_cast<std::streamsize>(file.size()));
 * ```
 *
 * @section Notes
 * - Copy operations are deleted to prevent duplicating large buffers.
 * - Move operations are defaulted.
 * - Use with `std::unique_ptr<AFile>` when large buffers are required to avoid stack overflow.
 * 
 * @section References
 * - Core::AFile
 * 
 */

namespace Core
{

class AFile
{
public:
    AFile()=default;
    virtual ~AFile()=default;

    // Delete copy
    AFile(const AFile&) = delete;
    AFile& operator=(const AFile&) = delete;

    AFile(AFile&&) noexcept = default;
    AFile& operator=(AFile&&) noexcept = default;

    /**
     * @brief Returns the underlying file buffer
     * 
     * @return Core::AVector<std::byte, FILE_MAX_SIZE>& 
     */
    inline Core::AVector<std::byte, FILE_MAX_SIZE>& buffer() noexcept 
    { 
        return buffer_;
    }

    /**
     * @brief Pointer to the raw buffer data
     * 
     * @return std::byte* 
     */
    inline std::byte* data() noexcept
    {
        return buffer_.data();
    }

    /**
     * @brief Number of valid bytes in the buffer.
     * 
     * @return std::size_t 
     */
    inline std::size_t size() const noexcept
    { 
        return buffer_.size(); 
    }

    /**
     * @brief Filesystem path of the loaded file
     * 
     * @return const std::filesystem::path& 
     */
    inline const std::filesystem::path& path() const noexcept
    { 
        return path_; 
    }

    /**
     * @brief Sets the path for the file object
     * 
     * @param path 
     */
    inline void setPath(const std::filesystem::path& path) noexcept
    { 
        path_ = path; 
    }

private:
    std::filesystem::path path_;
    Core::AVector<std::byte, FILE_MAX_SIZE> buffer_;
};

}// namespace Core
