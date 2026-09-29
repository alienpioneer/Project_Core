/**
 * @file test_posix_dir.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include <gtest/gtest.h>

#include "utils/PosixDir.hpp"

namespace Test_PosixDir
{

TEST(PosixDir, OpenValidDir)
{
    Utils::PosixDir dir("/tmp");
    EXPECT_TRUE(dir);
    EXPECT_GE(dir.fd(), 0);
}

TEST(PosixDir, OpenInvalidDir)
{
    Utils::PosixDir dir("/tmp/nonexistent_dir_123");
    EXPECT_FALSE(dir);
}

TEST(PosixDir, JoinPath)
{
    EXPECT_EQ(Utils::PosixDir::joinPath("/tmp", "file"), "/tmp/file");
    EXPECT_EQ(Utils::PosixDir::joinPath("/tmp/", "file"), "/tmp/file");
    EXPECT_EQ(Utils::PosixDir::joinPath("/tmp", "/file"), "/tmp/file");
}

TEST(PosixDir, GetPath)
{
    Utils::PosixDir dir("/tmp");
    EXPECT_TRUE(dir);
    EXPECT_GE(dir.fd(), 0);
    EXPECT_EQ(dir.path(), "/tmp");
}

}// namespace Test_PosixDir
