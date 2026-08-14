if(NOT DEFINED NGSTD_BUILD_DIRECTORY)
    message(FATAL_ERROR "NGSTD_BUILD_DIRECTORY is required")
endif()
if(NOT DEFINED NGSTD_SOURCE_DIRECTORY)
    message(FATAL_ERROR "NGSTD_SOURCE_DIRECTORY is required")
endif()
if(NOT DEFINED NGSTD_QT_PACKAGE_DIRECTORY)
    message(FATAL_ERROR "NGSTD_QT_PACKAGE_DIRECTORY is required")
endif()
if(NOT DEFINED NGSTD_QT_PACKAGE_NAME)
    message(FATAL_ERROR "NGSTD_QT_PACKAGE_NAME is required")
endif()

set(consumer_root "${NGSTD_BUILD_DIRECTORY}/install-consumer")
set(install_prefix "${consumer_root}/prefix")
set(consumer_build "${consumer_root}/build")
set(consumer_prefix_path "${install_prefix}")
if(NGSTD_CMAKE_PREFIX_PATH)
    list(APPEND consumer_prefix_path ${NGSTD_CMAKE_PREFIX_PATH})
endif()
string(REPLACE ";" "\\;" consumer_prefix_argument "${consumer_prefix_path}")

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

set(
    configure_command
    "${CMAKE_COMMAND}"
    -S "${NGSTD_SOURCE_DIRECTORY}/consumer"
    -B "${consumer_build}"
    "-DCMAKE_PREFIX_PATH:STRING=${consumer_prefix_argument}"
    -D${NGSTD_QT_PACKAGE_NAME}_DIR=${NGSTD_QT_PACKAGE_DIRECTORY}
)
if(NGSTD_CMAKE_TOOLCHAIN_FILE)
    list(APPEND configure_command
         -DCMAKE_TOOLCHAIN_FILE=${NGSTD_CMAKE_TOOLCHAIN_FILE})
endif()

file(REMOVE_RECURSE "${consumer_build}")

execute_process(
    COMMAND ${configure_command}
    RESULT_VARIABLE configure_result
)
if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR "Configuring the installed consumer failed")
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}" --build "${consumer_build}" --config Release
    RESULT_VARIABLE build_result
)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "Building the installed consumer failed")
endif()

set(consumer_suffix "${CMAKE_EXECUTABLE_SUFFIX}")
if(WIN32 AND NOT consumer_suffix)
    set(consumer_suffix ".exe")
endif()
set(consumer_executable "")
foreach(candidate
        "${consumer_build}/Release/ngstd_widgets_consumer${consumer_suffix}"
        "${consumer_build}/ngstd_widgets_consumer${consumer_suffix}")
    if(EXISTS "${candidate}")
        set(consumer_executable "${candidate}")
        break()
    endif()
endforeach()
if(NOT consumer_executable)
    message(FATAL_ERROR "Installed consumer executable is missing")
endif()
if(WIN32)
    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" -E env
            "PATH=${install_prefix}/bin\;$ENV{PATH}"
            "${consumer_executable}"
        TIMEOUT 15
        RESULT_VARIABLE run_result
    )
else()
    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" -E env
            "QT_QPA_PLATFORM=offscreen"
            "${consumer_executable}"
        TIMEOUT 15
        RESULT_VARIABLE run_result
    )
endif()
if(NOT run_result EQUAL 0)
    message(FATAL_ERROR "Running the installed consumer failed")
endif()
