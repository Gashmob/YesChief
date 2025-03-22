cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

# Nix dependencies
file(GLOB NIX_STORE_LIB LIST_DIRECTORIES true "/nix/store/*/lib/cmake/*")
message(DEBUG NIX_STORE_LIB=${NIX_STORE_LIB})
list(APPEND CMAKE_PREFIX_PATH ${NIX_STORE_LIB})
file(GLOB NIX_STORE_BIN LIST_DIRECTORIES true "/nix/store/*/bin/")
message(DEBUG NIX_STORE_BIN=${NIX_STORE_BIN})
list(APPEND CMAKE_PROGRAM_PATH ${NIX_STORE_BIN})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION LIB_VERSION)

project(
        yeschief
        VERSION ${LIB_VERSION}
        LANGUAGES CXX
)

include(FetchContent)
cmake_policy(SET CMP0135 NEW)

# _.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-.
# Library

## Coverage config
add_library(additional_config INTERFACE)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
if (CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # No optimization + debug info + coverage flags
    target_compile_options(additional_config INTERFACE -O0 -g --coverage)
    target_link_options(additional_config INTERFACE --coverage)
else ()
    # Full optimization
    target_compile_options(additional_config INTERFACE -O3)
endif ()

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS yeschief
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/**/*.cpp"
)
message(DEBUG SRC_FILES=${SRC_FILES})

add_library(yeschief ${SRC_FILES})

target_include_directories(yeschief PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>  # <prefix>/include
)

target_compile_definitions(yeschief
        PUBLIC LIB_VERSION="${LIB_VERSION}"
)

target_compile_options(yeschief PUBLIC -Wall)

# _.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-.
# Tests

option(BUILD_TESTING "Build the testing tree." OFF)
if (BUILD_TESTING AND (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    message(DEBUG "Build test tree")
    enable_testing()
    add_subdirectory(tests)
endif ()

# _.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-.
# Packaging

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(DEBUG "Build install tree")
    include(packaging)
    lib_package()
endif ()
