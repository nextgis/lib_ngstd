if(NOT DEFINED NGSTD_BUILD_DIRECTORY)
    message(FATAL_ERROR "NGSTD_BUILD_DIRECTORY is required")
endif()
if(NOT DEFINED NGSTD_SOURCE_DIRECTORY)
    message(FATAL_ERROR "NGSTD_SOURCE_DIRECTORY is required")
endif()

set(consumer_root "${NGSTD_BUILD_DIRECTORY}/install-consumer")
set(install_prefix "${consumer_root}/prefix")
set(consumer_build "${consumer_root}/build")

execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        --install "${NGSTD_BUILD_DIRECTORY}"
        --prefix "${install_prefix}"
    RESULT_VARIABLE install_result
)
if(NOT install_result EQUAL 0)
    message(FATAL_ERROR "Installing ngstd_widgets failed")
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        -S "${NGSTD_SOURCE_DIRECTORY}/consumer"
        -B "${consumer_build}"
        -DCMAKE_PREFIX_PATH=${install_prefix}
    RESULT_VARIABLE configure_result
)
if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR "Configuring the installed consumer failed")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${consumer_build}"
    RESULT_VARIABLE build_result
)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "Building the installed consumer failed")
endif()

set(consumer_executable
    "${consumer_build}/ngstd_widgets_consumer${CMAKE_EXECUTABLE_SUFFIX}")
if(WIN32)
    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" -E env
            "PATH=${install_prefix}/bin\;$ENV{PATH}"
            "${consumer_executable}"
        RESULT_VARIABLE run_result
    )
else()
    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" -E env
            "QT_QPA_PLATFORM=offscreen"
            "${consumer_executable}"
        RESULT_VARIABLE run_result
    )
endif()
if(NOT run_result EQUAL 0)
    message(FATAL_ERROR "Running the installed consumer failed")
endif()
