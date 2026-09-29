/**
 * @file test_socket.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include <gtest/gtest.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <unistd.h>

#include "utils/Socket.hpp"

namespace TestSocket
{

TEST(Socket, OpenClose)
{
    Utils::Socket s;

    EXPECT_FALSE(s.valid());

    auto ret = s.open(AF_INET, SOCK_STREAM, 0);
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(s.valid());
    EXPECT_GE(s.get(), 0);

    EXPECT_EQ(s.close(), 0);
    EXPECT_FALSE(s.valid());
}

TEST(Socket, DoubleOpenAutoClose)
{
    Utils::Socket s;

    ASSERT_EQ(s.open(AF_INET, SOCK_STREAM, 0), 0);
    EXPECT_TRUE(s.valid());

    ASSERT_EQ(s.open(AF_INET, SOCK_STREAM, 0), 0);
    EXPECT_TRUE(s.valid());

    EXPECT_GE(s.get(), 0);
}

// The same descriptor id can be reused !
TEST(Socket, DoubleOpenDoesNotLeak)
{
    Utils::Socket s;

    ASSERT_EQ(s.open(AF_INET, SOCK_STREAM, 0), 0);

    int fd1 = s.get();

    EXPECT_GE(fd1, 0);

    ASSERT_EQ(s.open(AF_INET, SOCK_STREAM, 0), 0);

    int fd2 = s.get();

    EXPECT_GE(fd2, 0);
    EXPECT_TRUE(s.valid());
}

TEST(Socket, SetNonBlocking)
{
    Utils::Socket s;
    ASSERT_EQ(s.open(AF_INET, SOCK_STREAM, 0), 0);

    EXPECT_EQ(s.setNonBlocking(true), 0);
    EXPECT_EQ(s.setNonBlocking(false), 0);
}

TEST(Socket, SetReuseAddr)
{
    Utils::Socket s;
    ASSERT_EQ(s.open(AF_INET, SOCK_STREAM, 0), 0);

    EXPECT_EQ(s.setReuseAddr(true), 0);
    EXPECT_EQ(s.setReuseAddr(false), 0);
}

TEST(Socket, BindInvalidFd)
{
    Utils::Socket s;

    sockaddr addr{};
    EXPECT_NE(s.bind(&addr, sizeof(addr)), 0);
}

TEST(Socket, BindInvalidAddress)
{
    Utils::Socket s;

    ASSERT_EQ(s.open(AF_INET, SOCK_STREAM, 0), 0);

    // Invalid: null pointer
    EXPECT_NE(s.bind(nullptr, sizeof(sockaddr)), 0);

    // Invalid: wrong length (too small)
    sockaddr addr{};
    EXPECT_NE(s.bind(&addr, 0), 0);
}

TEST(Socket, GetInvalid)
{
    Utils::Socket s;

    EXPECT_EQ(s.get(), -1);
    EXPECT_FALSE(s.valid());
}

}// namespace TestSocket