cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

find_package(SIP REQUIRED)
find_package(PyQt REQUIRED)
find_package(OpenSSL REQUIRED)

if(NOT OPENSSL_CRYPTO_LIBRARY)
    message(FATAL_ERROR "OpenSSL crypto library path is required for Python bindings")
endif()

if(SIP_VERSION_STR VERSION_LESS 6)
    message(FATAL_ERROR "Python bindings require SIP 6 or newer")
endif()

set(NGSTD_PYTHON_PROJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/sip-project")
set(NGSTD_PYTHON_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/package")
set(NGSTD_PYTHON_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/build")
set(NGSTD_PYPROJECT "${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml")

set(CONFIGURE_ARGS
    --verbose
    --build-dir "${NGSTD_PYTHON_BUILD_DIR}"
    --target-dir "${NGSTD_PYTHON_TARGET_DIR}"
    --no-docstrings
    --no-distinfo
)

if(TARGET ${QT_VERSION_BASE}::qmake)
    get_property(
        QT_QMAKE_EXECUTABLE
        TARGET ${QT_VERSION_BASE}::qmake
        PROPERTY IMPORTED_LOCATION
    )
else()
    if(NGSTD_QT_MAJOR STREQUAL "6")
        find_program(QT_QMAKE_EXECUTABLE NAMES qmake6 qmake-qt6 qmake REQUIRED)
    else()
        find_program(QT_QMAKE_EXECUTABLE NAMES qmake-qt5 qmake5 qmake REQUIRED)
    endif()
endif()

get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
get_filename_component(QT_ROOT_DIR "${QT_BIN_DIR}" DIRECTORY)
list(APPEND CONFIGURE_ARGS
    --qmake=${QT_QMAKE_EXECUTABLE}
    --qmake-setting "QMAKE_RPATHDIR+=${QT_ROOT_DIR}/lib"
    --qmake-setting INCLUDEPATH+=${PROJECT_SOURCE_DIR}/src
    --qmake-setting INCLUDEPATH+=${PROJECT_SOURCE_DIR}/src/core
    --qmake-setting INCLUDEPATH+=${PROJECT_SOURCE_DIR}/src/framework
)

foreach(QT_COMPONENT Core Gui Widgets)
    if(TARGET ${QT_VERSION_BASE}::${QT_COMPONENT})
        get_property(
            QT_INCLUDE_DIRS
            TARGET ${QT_VERSION_BASE}::${QT_COMPONENT}
            PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        )
        foreach(QT_INCLUDE_DIR ${QT_INCLUDE_DIRS})
            list(APPEND CONFIGURE_ARGS --qmake-setting INCLUDEPATH+=${QT_INCLUDE_DIR})
        endforeach()
    endif()
endforeach()

if(WIN32)
    string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

    foreach(QT_COMPONENT Core Gui Widgets)
        if(TARGET ${QT_VERSION_BASE}::${QT_COMPONENT})
            get_target_property(
                QT_COMPONENT_LIB
                ${QT_VERSION_BASE}::${QT_COMPONENT}
                IMPORTED_IMPLIB_${BUILD_TYPE}
            )
            if(QT_COMPONENT_LIB)
                list(APPEND CONFIGURE_ARGS --qmake-setting "LIBS+=${QT_COMPONENT_LIB}")
            endif()
        endif()
    endforeach()

endif()

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/pyproject.toml.in
    ${NGSTD_PYPROJECT}
    @ONLY
)

add_custom_target(build_py ALL
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${NGSTD_PYTHON_PROJECT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${NGSTD_PYTHON_PROJECT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${NGSTD_PYPROJECT}"
        "${CMAKE_CURRENT_SOURCE_DIR}/project.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
        "${NGSTD_PYTHON_PROJECT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/core"
        "${NGSTD_PYTHON_PROJECT_DIR}/core"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/framework"
        "${NGSTD_PYTHON_PROJECT_DIR}/framework"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/sip"
        "${NGSTD_PYTHON_PROJECT_DIR}/sip"
    COMMAND ${CMAKE_COMMAND} -E chdir
        "${NGSTD_PYTHON_PROJECT_DIR}"
        ${CMAKE_COMMAND}
        -E
        env
        "NGSTD_CORE_LINKER_FILE=$<TARGET_LINKER_FILE:ngstd_core>"
        "NGSTD_FRAMEWORK_LINKER_FILE=$<TARGET_LINKER_FILE:ngstd_framework>"
        "NGSTD_OPENSSL_CRYPTO_LINKER_FILE=${OPENSSL_CRYPTO_LIBRARY}"
        ${SIP_INSTALL_EXECUTABLE}
        ${CONFIGURE_ARGS}
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${NGSTD_PYTHON_TARGET_DIR}/ngstd/core"
        "${NGSTD_PYTHON_TARGET_DIR}/ngstd/framework"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
        "${NGSTD_PYTHON_TARGET_DIR}/ngstd/__init__.py"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/core/__init__.py"
        "${NGSTD_PYTHON_TARGET_DIR}/ngstd/core/__init__.py"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/framework/__init__.py"
        "${NGSTD_PYTHON_TARGET_DIR}/ngstd/framework/__init__.py"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS ngstd_core ngstd_framework
    VERBATIM
)
