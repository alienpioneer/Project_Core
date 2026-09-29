/**
 * @file test_file_descriptor.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include <gtest/gtest.h>

#include <unistd.h>
#include <fcntl.h>

#include "utils/FileDescriptor.hpp"

namespace Test_FileDescriptor
{

TEST(FileDescriptor, MoveSemantics)
{
    int fd = ::open("/dev/null", O_RDONLY);
    ASSERT_GE(fd, 0);

    Utils::FileDescriptor a(fd);
    Utils::FileDescriptor b(std::move(a));

    EXPECT_FALSE(a);
    EXPECT_TRUE(b);
}

TEST(FileDescriptor, Release)
{
    int fd = ::open("/dev/null", O_RDONLY);
    Utils::FileDescriptor f(fd);

    int raw = f.release();

    EXPECT_EQ(f.get(), -1);
    EXPECT_GE(raw, 0);

    ::close(raw);
}

}// namespace Test_FileDescriptor
