cmake_minimum_required(VERSION 3.21)
project(torustiq_sdk VERSION 0.1.0 LANGUAGES C CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Header-only SDK: a pure C API (sdk.h) plus optional C++ helpers (sdk.hpp).
add_library(torustiq_sdk INTERFACE)
add_library(torustiq::sdk ALIAS torustiq_sdk)
target_include_directories(torustiq_sdk INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# --- Install / find_package(torustiq_sdk) support ---
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS torustiq_sdk EXPORT torustiq_sdkTargets)
install(EXPORT torustiq_sdkTargets
    NAMESPACE torustiq::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/torustiq_sdk)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/torustiq_sdkConfig.cmake
    "include(\"\${CMAKE_CURRENT_LIST_DIR}/torustiq_sdkTargets.cmake\")\n")
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/torustiq_sdkConfigVersion.cmake
    COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/torustiq_sdkConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/torustiq_sdkConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/torustiq_sdk)

# --- Tests (only when built as the top-level project) ---
if(PROJECT_IS_TOP_LEVEL)
    option(TORUSTIQ_SDK_BUILD_TESTS "Build SDK header checks" ON)
else()
    option(TORUSTIQ_SDK_BUILD_TESTS "Build SDK header checks" OFF)
endif()
if(TORUSTIQ_SDK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
