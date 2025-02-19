# YesChief!

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Version](version.svg)

C++ library helping you to easily create manage options and commands of your CLI.

## Use it in your project

You can retrieve rpm/deb package attached to [latest release](https://github.com/Gashmob/YesChief/releases/latest).

Devel packages are also build for each push to master branch in [package workflow](https://github.com/Gashmob/YesChief/actions/workflows/package.yml?query=branch%3Amaster).

For CMake based projects, you can simply use `FetchContent` this way:

```cmake
FetchContent_Declare(
        yeschief
        GIT_REPOSITORY https://github.com/Gashmob/YesChief.git
        GIT_TAG v1.0.0
)
FetchContent_GetProperties(yeschief)

if(NOT yeschief_POPULATED)
    message(STATUS "Fetching yeschief...")
    FetchContent_Populate(yeschief)
    add_subdirectory(${yeschief_SOURCE_DIR} ${yeschief_BINARY_DIR})
endif()

target_link_libraries(my_target PUBLIC yeschief)
```

Then the include path is just `#include <yeschief.h>`

## Building

All following commands should be run inside a `nix-shell`, it allows you to have all needed tools and binary in your PATH.

Entire project is based on cmake. So you can just run cmake as for every cmake project.

Or you can run a pre-defined script:

```shell
build_release
```

## Testing

```shell
run_unit_tests
```

You can then get coverage of these tests with:

```shell
build_coverage
```
