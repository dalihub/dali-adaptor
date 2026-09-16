cmake_minimum_required(VERSION 3.10)
project(dali-gdbus-client-tests LANGUAGES CXX)

# Standalone tests: only GLib/GIO and dbus-daemon are needed, not a DALi build.
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0)
add_executable(test-gdbus-client test-gdbus-client.cpp)
target_compile_features(test-gdbus-client PRIVATE cxx_std_17)
target_compile_options(test-gdbus-client PRIVATE -Wall -Wextra -Werror)
target_include_directories(test-gdbus-client PRIVATE ../../..)
target_link_libraries(test-gdbus-client PRIVATE PkgConfig::GIO)

enable_testing()
add_test(NAME gdbus-client COMMAND test-gdbus-client)
set_tests_properties(gdbus-client PROPERTIES TIMEOUT 30)
