
# Build System

This project uses a unified build script supporting both **host** and **target** builds while keeping separate build directories for cache isolation.
The original target was a Yocto Scarthgap distribution but the project was adapted for Raspberry Pi.

Execute:

./build.sh [mode] [options]

- Default mode: `host`
- Modes:
  - `host`   → native build (tests, analysis, run supported)
  - `target` → cross-compilation using Yocto SDK

## Requirements

- Yocto SDK installed at:

/opt/seco-sdk-kirkstone/

The following tools must be installed on the host system:
- **CMake ≥ 3.22**
- **Ninja** (build system generator)
- **ccache** (compiler cache)
- **Doxygen** (only if using documentation generation)

Example (Debian/Ubuntu):
```
sudo apt install cmake ninja-build ccache doxygen cppcheck lcov
```

## Build Types

| Option  | CMake Type       | Notes                     |
|---------|------------------|---------------------------|
| debug   | Debug            | Full debug + logs         |
| release | Release          | Optimized, no debug       |
| reldeb  | RelWithDebInfo   | Balanced                  |
| min     | MinSizeRel       | Size optimized            |

---

## Help

./build.sh --help

---

## Host Build

### Features

- Static analysis
- Unit tests
- Stress tests
- Optional execution (`run.sh`)
- Optional Ninja generator

### Options

| Option   | Description                                   |
|----------|-----------------------------------------------|
| static   | Enable static analysis                        |
| unit     | Enable unit tests                             |
| stress   | Enable stress tests                           |
| all      | static + unit                                 |
| run      | Execute a basic test application after build  |
| ninja    | Use Ninja generator                           |
| doxygen  | Generate only tyhe doxygen documentation      |
| gcov     | Generate code coverage                        |
---

## Target Build (Yocto)

### Characteristics

- Uses Yocto SDK environment
- Cross-compilation (`aarch64-poky-linux-*`)
- No tests or execution
- Ninja always used

## Internal Behavior

### Host Mode

- Cleans `build/`
- Configures CMake with:
  - test flags
  - optional Ninja
- Builds
- Optionally runs:
  - static analysis
  - unit tests
  - executable
  - doxygen documentation generation
  - test coverage generation

Warning: The above mention optional actions are meant to work on the host only (standard usage guidelines), do not try to run them on target!

### Target Mode

- Sources Yocto SDK
- Sets cross-compilation environment
- Applies manual optimization flags
- Configures with Ninja
- Builds with verbose output

---

## Typical Workflows

### Development (Host)

```
./build.sh debug unit
```

### CI-like Check

```
./build.sh all ninja
```

### Cross Compilation

```
./build.sh target release
```

### Doxygen documentation

```
./build.sh doxygen
```

Output is generated using the configured `Doxyfile` into the `deploy/` folder.

---

### Coverage documentation using lcov

```
./build.sh gcov
```

Output is generated into the `deploy/coverage` folder. 

Warning: Due to testing conditions some tests may fail once. If so, relaunch the command.
Check if the tests really fail with:

```
./build.sh debug unit
```
It is not possible to test everything with gcov due to environment constraints!

---

## Deployment

All the files resulted from the build process are deploye to the local `deploy/` folder.

## Templates

To facilitate service creation a service template is provided in the `deploy/templates` folder.

Every time you modify these templates in the `examples/templates` fodler, you must rebuild the project to update them in the `deploy/templates` folder.

Remove methods that you don't override, change names and add in the ServiceTypes.hpp the corresponding service id and the corresponding Events in the EventTypes.hpp.
These two header files you must place in the include path on your project, DO NOT UPDATE them in the CORE project !

## Debugging compile time examples

```
source /opt/seco-sdk-kirkstone/environment-setup-cortexa53-crypto-poky-linux
```

Expand only headers:
```
time aarch64-poky-linux-g++   -mcpu=cortex-a53 -march=armv8-a+crc+crypto   -fstack-protector-strong   -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security   --sysroot=/opt/seco-sdk-kirkstone/sysroots/cortexa53-crypto-poky-linux   -DLOG_COMPILED_LEVEL=LOG_LEVEL_RELEASE   -I/home/rd2/__DEV__/Project_Core/include   -I/home/rd2/__DEV__/Project_Core/build-yocto/generated   -O3 -g0 -DNDEBUG   -Wall -Wextra -Wpedantic -std=gnu++17   -E /home/rd2/__DEV__/Project_Core/src/core/ServiceManager.cpp   -o /home/rd2/__DEV__/Project_Core/tmp
```

Compile without dependencies:
```
time aarch64-poky-linux-g++   -mcpu=cortex-a53 -march=armv8-a+crc+crypto   -fstack-protector-strong   -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security   --sysroot=/opt/seco-sdk-kirkstone/sysroots/cortexa53-crypto-poky-linux   -DLOG_COMPILED_LEVEL=LOG_LEVEL_RELEASE   -I/home/rd2/__DEV__/Project_Core/include   -I/home/rd2/__DEV__/Project_Core/build-yocto/generated   -O3 -g0 -DNDEBUG   -Wall -Wextra -Wpedantic -std=gnu++17   -c /home/rd2/__DEV__/Project_Core/src/core/ServiceManager.cpp   -o /home/rd2/__DEV__/Project_Core/tmp
```
