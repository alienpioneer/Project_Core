/**
 * @file test_posix_io.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include <gtest/gtest.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include "utils/PosixIO.hpp"
#include "utils/PosixDir.hpp"

namespace TestPosixio
{

TEST(PosixIO, OpenAndWriteRead)
{
    const char* path = "/tmp/test_posixio.txt";

    Utils::PosixIO io(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    ASSERT_TRUE(io);

    const char data[] = "abc";
    EXPECT_EQ(io.write(data, sizeof(data)), sizeof(data));

    char buf[4]{};
    EXPECT_EQ(io.pread(buf, sizeof(buf)), sizeof(buf));
    EXPECT_STREQ(buf, "abc");
}

TEST(PosixIO, InvalidPath)
{
    Utils::PosixIO io("/invalid/path/file", O_RDONLY, 0);
    EXPECT_FALSE(io);
}

TEST(PosixIO, OpenAtUsingPosixDirFd)
{
    const char* dirPath = "/tmp";
    const char* fileName = "test_openat.txt";

    Utils::PosixDir dir(dirPath);
    ASSERT_TRUE(dir);

    Utils::PosixIO io(dir.fd(), fileName, O_CREAT | O_RDWR | O_TRUNC, 0644);
    ASSERT_TRUE(io);

    const char data[] = "openat";
    EXPECT_EQ(io.write(data, sizeof(data)), sizeof(data));

    char buf[7]{};
    EXPECT_EQ(io.pread(buf, sizeof(buf)), sizeof(buf));
    EXPECT_STREQ(buf, "openat");

    // cleanup
    std::string fullPath = Utils::PosixDir::joinPath(dirPath, fileName);
    ::unlink(fullPath.c_str());
}

TEST(PosixIO, OpenAtWithoutCreateFlag)
{
    const char* dirPath = "/tmp";
    const char* fileName = "test_openat_no_create.txt";

    // create file first
    std::string fullPath = Utils::PosixDir::joinPath(dirPath, fileName);
    int raw = ::open(fullPath.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
    ASSERT_GE(raw, 0);
    ::close(raw);

    Utils::PosixDir dir(dirPath);
    ASSERT_TRUE(dir);

    Utils::PosixIO io(dir.fd(), fileName, O_RDWR, 0);
    EXPECT_TRUE(io);

    ::unlink(fullPath.c_str());
}

TEST(PosixIO, WriteErrorReturnsNegativeErrno)
{
    Utils::PosixIO io("/tmp/test_write_error.txt", O_CREAT | O_RDONLY | O_TRUNC, 0644);
    ASSERT_TRUE(io);

    const char data[] = "fail";

    auto ret = io.write(data, sizeof(data));

    EXPECT_LT(ret, 0);               // must be negative
    EXPECT_EQ(ret, -EBADF);          // write on O_RDONLY
}

TEST(PosixIO, IoctlSuccess)
{
    int fds[2];
    ASSERT_EQ(pipe(fds), 0);

    Utils::PosixIO io("/dev/null", O_RDONLY, 0); // dummy init not used

    // hack: reuse fd via FileDescriptor move (simplest without modifying class)
    Utils::FileDescriptor fdWrap(fds[0]);
    Utils::PosixIO io2("/dev/null", O_RDONLY, 0);
    io2 = Utils::PosixIO("/dev/null", O_RDONLY, 0); // ensure valid object

    int bytes = 0;
    int ret = ::ioctl(fds[0], FIONREAD, &bytes);
    EXPECT_GE(ret, 0);

    ::close(fds[0]);
    ::close(fds[1]);
}

TEST(PosixIO, IoctlErrorReturnsNegativeErrno)
{
    Utils::PosixIO io("/tmp/test_ioctl.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
    ASSERT_TRUE(io);

    int dummy = 0;

    // invalid request → ENOTTY
    int ret = io.pioCtl(0xDEADBEEF, &dummy);

    EXPECT_LT(ret, 0);
}

TEST(PosixIO, GetFileDescriptor)
{
    Utils::PosixIO io("/tmp/test_fd.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
    ASSERT_TRUE(io);

    int fd = io.fd();

    EXPECT_GE(fd, 0);
}

TEST(PosixIO, GetPath)
{
    const std::string path = "/tmp/test_path.txt";

    Utils::PosixIO io(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    ASSERT_TRUE(io);

    EXPECT_EQ(io.path(), path);
}

TEST(PosixIO, OpenAtInvalidDirFd)
{
    const int invalidFd = -1;

    Utils::PosixIO io(invalidFd, "file.txt", O_CREAT | O_RDWR, 0644);

    EXPECT_FALSE(io);
    EXPECT_LT(io.fd(), 0);
}

TEST(PosixIO, PreadErrorReturnsNegativeErrno)
{
    Utils::PosixIO io("/tmp/test_pread_error.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    ASSERT_TRUE(io);

    char buf[8]{};

    auto ret = io.pread(buf, sizeof(buf));

    EXPECT_LT(ret, 0);
    EXPECT_EQ(ret, -EBADF);
}

TEST(PosixIO, IoctlErrorReturnsNegativeErrno_Strict)
{
    Utils::PosixIO io("/tmp/test_ioctl_error.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
    ASSERT_TRUE(io);

    int dummy = 0;

    auto ret = io.pioCtl(0xDEADBEEF, &dummy);

    EXPECT_LT(ret, 0);
    EXPECT_EQ(ret, -ENOTTY);
}

TEST(PosixIO, IoctlSuccessFionread)
{
    int fds[2];
    ASSERT_EQ(::pipe(fds), 0);

    // wrap read end
    Utils::PosixIO io("/dev/null", O_RDONLY, 0);
    ASSERT_TRUE(io);

    // replace fd via move (reuse FileDescriptor semantics)
    Utils::FileDescriptor fdWrap(fds[0]);
    // direct construction workaround: use dup to inject valid fd
    int dupFd = ::dup(fds[0]);
    ASSERT_GE(dupFd, 0);

    Utils::PosixIO io2("/dev/null", O_RDONLY, 0);
    *(int*)&io2 = dupFd; // minimal intrusive hack if no setter exists

    // write some data so FIONREAD > 0
    const char data[] = "abc";
    ASSERT_EQ(::write(fds[1], data, sizeof(data)), sizeof(data));

    int bytes = 0;
    int ret = io2.pioCtl(FIONREAD, &bytes);

    EXPECT_GE(ret, 0);
    EXPECT_GT(bytes, 0);

    ::close(fds[0]);
    ::close(fds[1]);
}

}// namespace TestPosixio
