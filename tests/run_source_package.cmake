if(NOT DEFINED NGSTD_BUILD_DIRECTORY)
    message(FATAL_ERROR "NGSTD_BUILD_DIRECTORY is required")
endif()
if(NOT DEFINED NGSTD_SOURCE_DIRECTORY)
    message(FATAL_ERROR "NGSTD_SOURCE_DIRECTORY is required")
endif()
if(NOT DEFINED NGSTD_PACKAGE_NAME)
    message(FATAL_ERROR "NGSTD_PACKAGE_NAME is required")
endif()
if(NOT DEFINED NGSTD_QT_MAJOR)
    message(FATAL_ERROR "NGSTD_QT_MAJOR is required")
endif()
if(NOT DEFINED NGSTD_GENERATOR)
    message(FATAL_ERROR "NGSTD_GENERATOR is required")
endif()

set(package_name "${NGSTD_PACKAGE_NAME}")
set(package_path "${NGSTD_BUILD_DIRECTORY}/${package_name}.tar.gz")
set(test_directory "${NGSTD_BUILD_DIRECTORY}/source-package-test")
set(source_directory "${test_directory}/${package_name}")
set(build_directory "${test_directory}/build")

file(REMOVE_RECURSE "${test_directory}")
file(MAKE_DIRECTORY "${test_directory}")

execute_process(
    COMMAND
        "${CMAKE_CPACK_COMMAND}"
        --config "${NGSTD_BUILD_DIRECTORY}/CPackSourceConfig.cmake"
        -G TGZ
    WORKING_DIRECTORY "${NGSTD_BUILD_DIRECTORY}"
    RESULT_VARIABLE package_result
)
if(NOT package_result EQUAL 0)
    message(FATAL_ERROR "Source package generation failed")
endif()
if(NOT EXISTS "${package_path}")
    message(FATAL_ERROR "Source package was not created: ${package_path}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar xzf "${package_path}"
    WORKING_DIRECTORY "${test_directory}"
    RESULT_VARIABLE extract_result
)
if(NOT extract_result EQUAL 0)
    message(FATAL_ERROR "Source package extraction failed")
endif()

foreach(excluded_path old tasks build-qt5 build-qt6 .git .agents .codex)
    if(EXISTS "${source_directory}/${excluded_path}")
        message(FATAL_ERROR "Excluded path is packaged: ${excluded_path}")
    endif()
endforeach()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        -S "${source_directory}"
        -B "${build_directory}"
        -G "${NGSTD_GENERATOR}"
        -DNGSTD_QT_MAJOR=${NGSTD_QT_MAJOR}
        -DBUILD_CORE=OFF
        -DBUILD_FRAMEWORK=OFF
        -DBUILD_WIDGETS=ON
        -DBUILD_SHARED_LIBS=ON
        -DBUILD_TESTING=ON
        -DNGSTD_WIDGETS_BUILD_GALLERY=OFF
        -DNGSTD_ENABLE_PACKAGING_TESTS=OFF
        -DCMAKE_DISABLE_FIND_PACKAGE_Python3=TRUE
    RESULT_VARIABLE configure_result
)
if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR "Packaged source configuration failed")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${build_directory}" --parallel 2
    RESULT_VARIABLE build_result
)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "Packaged source build failed")
endif()

execute_process(
    COMMAND
        "${CMAKE_CTEST_COMMAND}"
        --test-dir "${build_directory}"
        --output-on-failure
    RESULT_VARIABLE test_result
)
if(NOT test_result EQUAL 0)
    message(FATAL_ERROR "Packaged source tests failed")
endif()
