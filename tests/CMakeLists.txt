if(BUILD_WIDGETS)
    add_subdirectory(widgets)
endif()

if(NGSTD_ENABLE_PACKAGING_TESTS)
    add_test(
        NAME ngstd_source_package
        COMMAND
            ${CMAKE_COMMAND}
            -DNGSTD_BUILD_DIRECTORY=${CMAKE_BINARY_DIR}
            -DNGSTD_SOURCE_DIRECTORY=${CMAKE_SOURCE_DIR}
            -DNGSTD_PACKAGE_NAME=ngstd-${VERSION}
            -DNGSTD_QT_MAJOR=${NGSTD_QT_MAJOR}
            -DNGSTD_GENERATOR=${CMAKE_GENERATOR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_source_package.cmake
    )
endif()
