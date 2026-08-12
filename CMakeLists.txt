cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(ngstd LANGUAGES CXX)

set(NGSTD_QT_MAJOR "5" CACHE STRING "Qt major version used to build ngstd")
set_property(CACHE NGSTD_QT_MAJOR PROPERTY STRINGS 5 6)
option(WITH_BINDINGS "Build Python bindings" OFF)
option(WITH_SENTRY "Build Sentry logger support" OFF)
option(BUILD_CORE "Build ngstd core library" ON)
option(BUILD_FRAMEWORK "Build ngstd framework library" ON)
option(BUILD_WIDGETS "Build ngstd Qt Widgets library" ON)
option(NGSTD_WIDGETS_BUILD_GALLERY "Build ngstd widgets gallery" OFF)
option(BUILD_TESTING "Build ngstd tests" OFF)
option(NGSTD_WIDGETS_WARNINGS_AS_ERRORS "Treat widget warnings as errors" OFF)
option(NGSTD_WIDGETS_ENABLE_SANITIZERS "Enable widget ASan and UBSan" OFF)
option(NGSTD_WIDGETS_ENABLE_COVERAGE "Enable widget coverage" OFF)
option(NGSTD_ENABLE_PACKAGING_TESTS "Test the generated source package" OFF)

set(CMAKE_COLOR_MAKEFILE ON)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(util)

ngstd_read_version(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/version.h"
    VERSION
    NGLIB_MAJOR_VERSION
    NGLIB_MINOR_VERSION
    NGLIB_PATCH_NUMBER
)
message(STATUS "ngstd version ${VERSION}")

set(NGSTD_PLUGIN_LIB_DIR "lib" CACHE STRING "Relative Qt plugin library directory")

if(NOT NGSTD_QT_MAJOR MATCHES "^(5|6)$")
    message(FATAL_ERROR "NGSTD_QT_MAJOR must be 5 or 6")
endif()

if(NGSTD_QT_MAJOR STREQUAL "6")
    set(QT_VERSION_BASE Qt6)
else()
    set(QT_VERSION_BASE Qt5)
endif()
set(NGSTD_WIDGETS_QT_PACKAGE ${QT_VERSION_BASE})

if(BUILD_FRAMEWORK AND NOT BUILD_CORE)
    message(FATAL_ERROR "BUILD_FRAMEWORK requires BUILD_CORE")
endif()

if(WITH_BINDINGS AND (NOT BUILD_CORE OR NOT BUILD_FRAMEWORK))
    message(FATAL_ERROR "WITH_BINDINGS requires BUILD_CORE and BUILD_FRAMEWORK")
endif()

if(WITH_SENTRY AND NOT BUILD_FRAMEWORK)
    message(FATAL_ERROR "WITH_SENTRY requires BUILD_FRAMEWORK")
endif()

if(NGSTD_WIDGETS_BUILD_GALLERY AND NOT BUILD_WIDGETS)
    message(FATAL_ERROR "NGSTD_WIDGETS_BUILD_GALLERY requires BUILD_WIDGETS")
endif()

if(POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

if(WITH_BINDINGS)
    find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

    set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
endif()

set(NGSTD_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(NGLIBS)
if(BUILD_CORE)
    list(APPEND NGLIBS core)
endif()
if(BUILD_FRAMEWORK)
    list(APPEND NGLIBS framework)
endif()
if(BUILD_WIDGETS)
    list(APPEND NGLIBS widgets)
endif()

set(NGSTD_QT_COMPONENTS Core)
if(BUILD_CORE OR BUILD_FRAMEWORK OR WITH_BINDINGS)
    list(APPEND NGSTD_QT_COMPONENTS Network)
endif()
if(BUILD_FRAMEWORK OR BUILD_WIDGETS OR WITH_BINDINGS)
    list(APPEND NGSTD_QT_COMPONENTS Gui Widgets)
endif()
if(BUILD_WIDGETS)
    list(APPEND NGSTD_QT_COMPONENTS Svg)
endif()
if(BUILD_CORE OR BUILD_FRAMEWORK)
    list(APPEND NGSTD_QT_COMPONENTS LinguistTools)
endif()
if(BUILD_FRAMEWORK)
    list(APPEND NGSTD_QT_COMPONENTS Concurrent)
endif()
if(BUILD_TESTING AND BUILD_WIDGETS)
    list(APPEND NGSTD_QT_COMPONENTS Test)
endif()
list(REMOVE_DUPLICATES NGSTD_QT_COMPONENTS)

if(NGSTD_QT_MAJOR STREQUAL "6")
    find_package(Qt6 6.2 REQUIRED COMPONENTS ${NGSTD_QT_COMPONENTS})
    message(STATUS "Building with Qt6: ${Qt6Core_VERSION}")
else()
    find_package(Qt5 5.15 REQUIRED COMPONENTS ${NGSTD_QT_COMPONENTS})
    message(STATUS "Building with Qt5: ${Qt5Core_VERSION_STRING}")
endif()

if(BUILD_TESTING)
    enable_testing()
endif()

add_subdirectory(src)

if(NGSTD_WIDGETS_BUILD_GALLERY)
    add_subdirectory(examples/widgets_gallery)
endif()

if(WITH_BINDINGS)
    add_subdirectory(python)
endif()

if(BUILD_TESTING AND BUILD_WIDGETS)
    add_subdirectory(tests)
endif()

if(NGSTD_ENABLE_PACKAGING_TESTS)
    set(CPACK_SOURCE_GENERATOR "TGZ")
    set(CPACK_SOURCE_PACKAGE_FILE_NAME "ngstd-${VERSION}")
    set(CPACK_SOURCE_IGNORE_FILES
        "/\\.agents/"
        "/\\.codex/"
        "/\\.git/"
        "/\\.ruff_cache/"
        "/\\.venv/"
        "/\\.vscode/"
        "/build[^/]*/"
        "/out/"
    )
    include(CPack)
endif()
