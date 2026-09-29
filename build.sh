#!/bin/bash

# @file build.sh
# @author Alexandru ALEXANDRESCU
# All rights reserved.

# Exit immediately if a command exits with a non-zero status
set -e

MODE="host"   # host | target

# Defaults (host)
BUILD_DIR="build"
COVERAGE_BUILD_DIR="build-coverage"
BUILD_TYPE="Release"

# Host-only features
GCOV_OPT="OFF"
STATIC_OPT="OFF"
UNIT_OPT="OFF"
STRESS_OPT="OFF"
USE_NINJA="OFF"
DOXYGEN=false
RUN=false

TOOLCHAIN_FILE="cmake/toolchain-rpi.cmake"

# Parse args
for arg in "$@"
do
    case "$arg" in
        # Documentation only
        "doxygen")DOXYGEN=true ;;
        "gcov")   GCOV_OPT="ON" ;;

        # Mode
        "target") MODE="target" ;;
        "host")   MODE="host" ;;

        # Common build types
        "debug")  BUILD_TYPE="Debug" ;;
        "release")BUILD_TYPE="Release" ;;
        "reldeb") BUILD_TYPE="RelWithDebInfo" ;;
        "min")    BUILD_TYPE="MinSizeRel" ;;

        # Host-only features
        "run")    RUN=true ;;
        "static") STATIC_OPT="ON" ;;
        "unit")   UNIT_OPT="ON" ;;
        "stress") STRESS_OPT="ON"; UNIT_OPT="ON" ;;
        "all")    STATIC_OPT="ON"; UNIT_OPT="ON" ;;
        "ninja")  USE_NINJA="ON" ;;

        "h"|"--help")
            echo "Usage:"
            echo "  ./build.sh [doxygen] [host|target] [options]"
            echo ""
            echo "Host options:"
            echo "  static, unit, stress, all, run, ninja"
            echo ""
            echo "Build types:"
            echo "  debug, release, reldeb, min - default Release"
            echo ""
            echo "Documentation only:"
            echo "  doxygen"
            exit 0 ;;

        *)
            echo "Unknown argument: $arg (skipping)" ;;
    esac
done

if $DOXYGEN; then
    mkdir -p deploy
    doxygen Doxyfile
    exit 0
fi

if [ "$GCOV_OPT" = "ON" ]; then
    BUILD_DIR="$COVERAGE_BUILD_DIR"
    BUILD_TYPE="Debug"
fi

# -------------------------
# MODE SWITCH
# -------------------------

if [ "$MODE" = "target" ]; then

    BUILD_DIR="build-target"

    echo "------------------------------------------"
    echo "BUILD CONFIGURATION:"
    echo "  Mode:   TARGET"
    echo "  Type:   $BUILD_TYPE"
    echo "------------------------------------------"

    # Important: Avoid full debug -g flag, unless absolutely needed. Will explode the compile time.
    if [ "$BUILD_TYPE" = "Release" ]; then
        export CXXFLAGS="-O3 -g0 -DNDEBUG"
        export CFLAGS="-O3 -g0 -DNDEBUG"
    fi

    if [ "$BUILD_TYPE" = "RelWithDebInfo" ]; then
        export CXXFLAGS="-O1 -g1 -DNDEBUG"
        export CFLAGS="-O1 -g1 -DNDEBUG"
    fi

    cmake -S . -B "$BUILD_DIR" \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -G Ninja \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

    /usr/bin/time -f "Total build time: %E" \
    cmake --build "$BUILD_DIR" --parallel 8 --verbose

    exit 0
fi

# -------------------------
# HOST BUILD (default)
# -------------------------

# rm -rf "$BUILD_DIR"

if [ "$GCOV_OPT" = "ON" ]; then
    echo "--- Clean build folder ---"
    if [ -d "$BUILD_DIR" ]; then
        if [ "$USE_NINJA" = "ON" ]; then
            ninja -C "$BUILD_DIR" -t clean || true
        else
            rm -rf "$BUILD_DIR"
        fi
    fi
fi

echo "------------------------------------------"
echo "BUILD CONFIGURATION:"
echo "  Mode:   HOST"
echo "  Type:   $BUILD_TYPE"
echo "  Static: $STATIC_OPT"
echo "  Unit:   $UNIT_OPT"
echo "  Stress: $STRESS_OPT"
echo "  Ninja:  $USE_NINJA"
echo "------------------------------------------"

if [ "$USE_NINJA" = "ON" ]; then
    cmake -S . -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DRUN_STATIC_ANALYSIS="$STATIC_OPT" \
        -DRUN_UNIT_TESTS="$UNIT_OPT" \
        -DRUN_STRESS_TESTS="$STRESS_OPT" \
        -DCODE_COVERAGE="$GCOV_OPT" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -G Ninja \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
else
    cmake -S . -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DRUN_STATIC_ANALYSIS="$STATIC_OPT" \
        -DRUN_UNIT_TESTS="$UNIT_OPT" \
        -DRUN_STRESS_TESTS="$STRESS_OPT" \
        -DCODE_COVERAGE="$GCOV_OPT" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

/usr/bin/time -f "Total build time: %E" \
cmake --build "$BUILD_DIR" --parallel 8
# cmake --build "$BUILD_DIR" --parallel 8 --verbose
# cmake --build "$BUILD_DIR" --parallel 8 --clean-first
# ninja -C "$BUILD_DIR" -j 8 -t clean && ninja -C "$BUILD_DIR" -j 8

if [ "$STATIC_OPT" = "ON" ]; then
    echo "--- Running Static Analysis ---"
    cmake --build "$BUILD_DIR" --target test_cppcheck
fi

if [ "$UNIT_OPT" = "ON" ]; then
    echo "--- Running Unit Tests ---"
    cmake --build "$BUILD_DIR" --target run_unit_tests
fi

if [ "$GCOV_OPT" = "ON" ]; then
    echo "--- Generating coverage ---"
    cmake --build "$BUILD_DIR" --target coverage
fi

if $RUN; then
    source run.sh
fi
