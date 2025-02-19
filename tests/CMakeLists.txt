FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 6910c9d9165801d8827d628cb72eb7ea9dd538c5 #v1.16.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# _.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-.
# Unit tests

file(GLOB_RECURSE UNIT_TEST_FILES
        "${PROJECT_SOURCE_DIR}/tests/unit/*.cpp"
        "${PROJECT_SOURCE_DIR}/tests/unit/**/*.cpp"
        "${PROJECT_SOURCE_DIR}/tests/unit/*.h"
        "${PROJECT_SOURCE_DIR}/tests/unit/**/*.h"
)
message(DEBUG UNIT_TEST_FILES=${UNIT_TEST_FILES})

add_executable(unit-tests ${UNIT_TEST_FILES})
target_include_directories(unit-tests PUBLIC
        "${PROJECT_SOURCE_DIR}/tests/unit")
target_compile_definitions(unit-tests PUBLIC FIXTURES_PATH="${PROJECT_SOURCE_DIR}/tests/unit/Fixtures")

target_link_libraries(unit-tests PRIVATE gtest_main gtest gmock yeschief)

gtest_discover_tests(unit-tests
        PROPERTIES LABELS "unit"
)
