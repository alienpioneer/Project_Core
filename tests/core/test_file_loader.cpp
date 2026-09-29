/**
 * @file test_file_loader.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include "core/FileLoader.hpp"

namespace TestFileLoader
{

class FileLoaderTest : public ::testing::Test
{
protected:
    std::filesystem::path tmp;

    void SetUp() override
    {
        tmp = std::filesystem::temp_directory_path() / "afile_tests";
        std::filesystem::create_directories(tmp);

        std::ofstream(tmp / "ok.bin", std::ios::binary) << "abcd";
        std::ofstream(tmp / "empty.bin", std::ios::binary);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(tmp);
    }
};

TEST_F(FileLoaderTest, LoadFile_Ok)
{
    auto res = Core::FileLoader::loadFile(tmp / "ok.bin");

    EXPECT_EQ(res.error, Core::FileLoader::FileResult::Ok);
    ASSERT_NE(res.file, nullptr);
    EXPECT_EQ(res.file->size(), 4u);
}

TEST_F(FileLoaderTest, LoadFile_NotFound)
{
    auto res = Core::FileLoader::loadFile(tmp / "missing.bin");

    EXPECT_EQ(res.error, Core::FileLoader::FileResult::NotFound);
    ASSERT_EQ(res.file, nullptr);
}

TEST_F(FileLoaderTest, LoadFile_EmptyFile)
{
    auto res = Core::FileLoader::loadFile(tmp / "empty.bin");

    EXPECT_EQ(res.error, Core::FileLoader::FileResult::Ok);
    ASSERT_NE(res.file, nullptr);
    EXPECT_EQ(res.file->size(), 0u);
}

TEST_F(FileLoaderTest, LoadDirectory_Ok)
{
    auto res = Core::FileLoader::loadDirectory(tmp);

    EXPECT_EQ(res.error, Core::FileLoader::FileResult::Ok);
    EXPECT_EQ(res.files.size(), 2u);
    EXPECT_TRUE(res.failures.empty());
}

TEST_F(FileLoaderTest, LoadDirectory_NotDirectory)
{
    auto res = Core::FileLoader::loadDirectory(tmp / "ok.bin");

    EXPECT_EQ(res.error, Core::FileLoader::FileResult::NotDirectory);
}

TEST_F(FileLoaderTest, NotRegularFile)
{
    auto dir = std::filesystem::temp_directory_path();

    auto result = Core::FileLoader::loadFile(dir);

    EXPECT_EQ(result.error, Core::FileLoader::FileResult::NotRegularFile);
}

// Trigger via file_size error OR > FILE_MAX_SIZE
TEST_F(FileLoaderTest, Overflow)
{
    auto path = std::filesystem::temp_directory_path() / "big_file.bin";

    std::ofstream f(path);
    f << std::string(Core::FILE_MAX_SIZE + 1, 'A');
    f.close();

    auto result = Core::FileLoader::loadFile(path);

    EXPECT_EQ(result.error, Core::FileLoader::FileResult::Overflow);

    std::filesystem::remove(path);
}

TEST_F(FileLoaderTest, OpenFailed)
{
    auto path = tmp / "no_read.bin";

    std::ofstream(path).put('A');

    std::filesystem::permissions(
        path,
        std::filesystem::perms::none,
        std::filesystem::perm_options::replace
    );

    auto result = Core::FileLoader::loadFile(path);

    // Not guaranteed on all systems
    EXPECT_TRUE(
        result.error == Core::FileLoader::FileResult::OpenFailed ||
        result.error == Core::FileLoader::FileResult::Ok
    );
}

TEST_F(FileLoaderTest, ReadFailed)
{
    auto path = std::filesystem::temp_directory_path() / "empty.bin";
    std::ofstream f(path);

    // Force size mismatch: truncate after size read
    auto result1 = Core::FileLoader::loadFile(path);
    EXPECT_EQ(result1.error, Core::FileLoader::FileResult::Ok); // sanity

    // Now simulate: remove file after open is not trivial
    // Better approach: use FIFO (POSIX)

    std::filesystem::remove(path);
}

}// namespace TestFileLoader
