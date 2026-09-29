# @file toolchain-rpi.cmake
# @author Alexandru ALEXANDRESCU
# All rights reserved.

cmake_minimum_required(VERSION 3.22)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

set(CMAKE_SYSROOT /opt/rpi-sysroot)

set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Force crt*.o, libc, libm, ld.so etc. to resolve from the sysroot
# instead of the cross-toolchain's bundled (older) glibc.
# -B: searched before GCC's built-in exec-prefix startfile dirs (fixes crt1.o/crti.o/crtn.o)
# -L: explicit, searched before the driver's implicit -L to the bundled libc dir
set(_rpi_libdir_flags
    "-B${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu"
    "-B${CMAKE_SYSROOT}/lib/aarch64-linux-gnu"
    "-L${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu"
    "-L${CMAKE_SYSROOT}/lib/aarch64-linux-gnu"
)

string(REPLACE ";" " " _rpi_libdir_flags "${_rpi_libdir_flags}")

set(CMAKE_C_FLAGS_INIT   "${_rpi_libdir_flags}")
set(CMAKE_CXX_FLAGS_INIT "${_rpi_libdir_flags}")
set(CMAKE_EXE_LINKER_FLAGS_INIT    "${_rpi_libdir_flags}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_rpi_libdir_flags}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${_rpi_libdir_flags}")

set(ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}")
set(ENV{PKG_CONFIG_LIBDIR}
    "${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig"
)
set(ENV{PKG_CONFIG_PATH} "")

# Debian ARM64 multiarch library directory
set(CMAKE_LIBRARY_PATH
    "${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu"
    "${CMAKE_SYSROOT}/lib/aarch64-linux-gnu"
)