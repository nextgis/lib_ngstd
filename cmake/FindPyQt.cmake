if(PyQt_FOUND)
    return()
endif()

if(NOT PYTHON_EXECUTABLE)
    set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
endif()

if(QT_VERSION_BASE STREQUAL "Qt6")
    set(_PYQT_PACKAGE_NAME PyQt6)
    set(_PYQT_PYUIC_NAMES pyuic6)
else()
    set(_PYQT_PACKAGE_NAME PyQt5)
    set(_PYQT_PYUIC_NAMES pyuic5)
endif()

execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c
        "import os; import ${_PYQT_PACKAGE_NAME}; from ${_PYQT_PACKAGE_NAME}.QtCore import PYQT_VERSION_STR; print(PYQT_VERSION_STR); print(os.path.dirname(${_PYQT_PACKAGE_NAME}.__file__))"
    RESULT_VARIABLE PYQT_RESULT
    OUTPUT_VARIABLE PYQT_OUTPUT
    ERROR_VARIABLE PYQT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(PYQT_RESULT EQUAL 0)
    string(REPLACE "\n" ";" PYQT_OUTPUT_LIST "${PYQT_OUTPUT}")
    list(GET PYQT_OUTPUT_LIST 0 PYQT_VERSION_STR)
    list(GET PYQT_OUTPUT_LIST 1 PYQT_MOD_DIR)
    set(PYQT_SIP_DIR "${PYQT_MOD_DIR}/bindings")
    find_program(PYQT_PYUIC_EXECUTABLE NAMES ${_PYQT_PYUIC_NAMES})
    if(PYQT_PYUIC_EXECUTABLE)
        get_filename_component(PYQT_BIN_DIR "${PYQT_PYUIC_EXECUTABLE}" DIRECTORY)
    endif()
endif()

if(PYQT_VERSION_STR AND EXISTS "${PYQT_SIP_DIR}")
    set(PyQt_FOUND TRUE)
    set(PYQT_FOUND TRUE)
    set(PYQT_PACKAGE_NAME ${_PYQT_PACKAGE_NAME})
    string(SUBSTRING "${PYQT_VERSION_STR}" 0 1 PYQT_VERSION_MAJOR)

    if(NOT PyQt_FIND_QUIETLY)
        message(STATUS "Found ${PYQT_PACKAGE_NAME} version: ${PYQT_VERSION_STR}")
    endif()
else()
    set(PyQt_FOUND FALSE)
    set(PYQT_FOUND FALSE)
    if(PyQt_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find ${_PYQT_PACKAGE_NAME}: ${PYQT_ERROR}")
    endif()
endif()
