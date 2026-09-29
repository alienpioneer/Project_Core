/**
 * @file FileLoader.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/FileLoader.hpp"
#include <utility>
#include <memory>
#include <fstream>


namespace Core
{
namespace FileLoader
{

FileLoadResult loadFile(const std::filesystem::path& path)
{
    FileLoadResult result;

    // local object, RAII applies
    auto afile = std::make_unique<AFile>();   // Heap, no stack overflow !!!

    std::error_code error_code;

    if (!std::filesystem::exists(path))
    {
        result.error = FileResult::NotFound;
        return result;
    }

    auto canonicalPath = std::filesystem::canonical(path, error_code);

    if (error_code)
    {
        result.error = FileResult::NotCanonicalPath;
        return result;
    }
     
    if (!std::filesystem::is_regular_file(canonicalPath))
    {
        result.error = FileResult::NotRegularFile;
        return result;
    }
        
    const auto size = std::filesystem::file_size(canonicalPath, error_code);

    if (error_code)
    {
        result.error = FileResult::Overflow;
        return result;
    }

    afile->setPath(canonicalPath);

    if (size > FILE_MAX_SIZE)
    {
        result.error = FileResult::Overflow;
        return result;
    }

    std::ifstream fstream(afile->path(), std::ios::binary);

    if (!fstream)
    {
        result.error = FileResult::OpenFailed;
        return result;
    } 

    afile->buffer().resize(size);
    if (!fstream.read(reinterpret_cast<char*>(afile->data()), static_cast<std::streamsize>(size)))
    {
        result.error = FileResult::ReadFailed;
        return result;
    }
        
    result.error = FileResult::Ok;
    result.file = std::move(afile);
    return result;
}

DirectoryLoadResult loadDirectory(const std::filesystem::path& dir)
{
    DirectoryLoadResult result;

    if (!std::filesystem::is_directory(dir))
    {
        result.error = FileResult::NotDirectory;
        return result;
    }

    for (const auto& entry : std::filesystem::directory_iterator(dir))
    {

        auto loadResult = loadFile(entry.path());

        if (loadResult.error == FileResult::Ok)
        {
            result.files.push_back(std::move(loadResult));
        }
        else
        {
            std::pair<std::filesystem::path, FileResult> failResult{entry.path(), loadResult.error};
            result.failures.push_back(failResult);
        }
    }

    return result;
}

} // namespace FileLoader
}// namespace Core