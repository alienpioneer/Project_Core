/**
 * @file FileLoader.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once
#include <optional>
#include "AFile.hpp"
#include "AVector.hpp"
#include "Config.hpp"

/**
 * @page FileLoaderPage File Loader
 * 
 * @brief Helper class to load file and directories by respecting the RAII principle.
 *
 * @section Overview
 * 
 * The File Loader module provides functions to load single files and directories
 * into memory using the Core::AFile class. It handles large fixed-size buffers
 * safely and reports errors through Core::FileResult.
 * 
 * @section Definitions
 * A canonical path is the unique, fully resolved, absolute path that the OS considers the real location of a file
 * Ex: A canonical path resolves: /etc/../etc/passwd or /tmp/link_to_passwd to /etc/passwd
 * 
 * A regular file is NOT a directory, a block device, a character device, symlink, etc.
 *
 * @section Classes
 * - Core::AFile : Represents a loaded file with a fixed-size buffer.
 * - Core::FileLoader::FileLoadResult : Result of loading a single file.
 * - Core::FileLoader::DirectoryLoadResult : Result of loading a directory of files.
 *
 * @section Usage
 * Example of loading a single file:
 * @code{.cpp}
 * auto result = Core::FileLoader::loadFile("/path/to/file.bin");
 * if (result.error == Core::FileResult::Ok)
 * {
 *     auto& file = result.file;
 *     // Use file->data() and file->size()
 * }
 * @endcode
 *
 * Example of loading a directory:
 * @code{.cpp}
 * auto dirResult = Core::FileLoader::loadDirectory("/path/to/dir");
 * for (auto& f : dirResult.files)
 * {
 *     // f.file points to a valid Core::AFile
 * }
 * for (auto& [path, fail] : dirResult.failures)
 * {
 *     // Handle failed loads
 * }
 * @endcode
 *
 * @section Notes
 * - All file loading is synchronous.
 * - Large fixed-size buffers are allocated inside AFile.
 * - Use std::unique_ptr<AFile> to avoid stack overflow.
 * 
 *  @section References
 * 
 * - Core::FileLoader::loadFile()
 * - Core::FileLoader::loadDirectory()
 * - @ref AFilePage
 */

namespace Core
{
namespace FileLoader
{

enum class FileResult
{
    Ok,
    NotFound,
    NotRegularFile,
    NotCanonicalPath,  
    NotDirectory,
    OpenFailed,
    ReadFailed,
    Overflow
};

/**
 * @brief The main type resulted from a file load operation
 * @note IMPORTANT : use unique_ptr to prevent stack overflow due to high preallocation
 */
struct FileLoadResult
{
    FileResult error;
    std::unique_ptr<AFile> file;
};

/**
 * @brief The main type resulted from a directory load operation
 * 
 */
struct DirectoryLoadResult
{
    FileResult error = FileResult::Ok;
    AVector<FileLoadResult, MAX_DIRECTORY_TREE> files;
    Core::AVector<std::pair<std::filesystem::path, FileResult>, MAX_DIRECTORY_TREE> failures;
};

/**
 * @brief Load a file
 * 
 * @param path 
 * @return FileLoadResult 
 */
FileLoadResult loadFile(const std::filesystem::path& path);

/**
 * @brief Load a directory
 * 
 * @param dir 
 * @return DirectoryLoadResult 
 */
[[maybe_unused]]
DirectoryLoadResult loadDirectory(const std::filesystem::path& dir);

} // namespace FileLoader

}// namespace Core