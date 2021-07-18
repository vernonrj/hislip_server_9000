project(hislip_server)
cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
    googletest 
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(hislip_server src/main.cpp)
target_include_directories(hislip_server PUBLIC src)
target_link_libraries(hislip_server wsock32 ws2_32)

enable_testing()

add_executable(
    the_tests
    test/test_stuff.cpp
)
target_include_directories(
    the_tests
    PUBLIC
    src/
)
target_link_libraries(
    the_tests
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(the_tests)