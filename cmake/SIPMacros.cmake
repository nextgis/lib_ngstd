# Macros for SIP
# ~~~~~~~~~~~~~~
# Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# SIP website: http://www.riverbankcomputing.co.uk/sip/index.php
#
# This file defines the following macros:
#
# ADD_SIP_PYTHON_MODULE (MODULE_NAME MODULE_SIP [library1, libaray2, ...])
#     Specifies a SIP file to be built into a Python module and installed.
#     MODULE_NAME is the name of Python module including any path name. (e.g.
#     os.sys, Foo.bar etc). MODULE_SIP the path and filename of the .sip file
#     to process and compile. libraryN are libraries that the Python module,
#     which is typically a shared library, should be linked to. The built
#     module will also be install into Python's site-packages directory.
#
# The behaviour of the ADD_SIP_PYTHON_MODULE macro can be controlled by a
# number of variables:
#
# SIP_INCLUDES - List of directories which SIP will scan through when looking
#     for included .sip files. (Corresponds to the -I option for SIP.)
#
# SIP_TAGS - List of tags to define when running SIP. (Corresponds to the -t
#     option for SIP.)
#
# SIP_CONCAT_PARTS - An integer which defines the number of parts the C++ code
#     of each module should be split into. Defaults to 8. (Corresponds to the
#     -j option for SIP.)
#
# SIP_DISABLE_FEATURES - List of feature names which should be disabled
#     running SIP. (Corresponds to the -x option for SIP.)
#
# SIP_EXTRA_OPTIONS - Extra command line options which should be passed on to
#     SIP.

set(SIP_INCLUDES)
set(SIP_TAGS)
set(SIP_CONCAT_PARTS 8)
set(SIP_DISABLE_FEATURES)
set(SIP_EXTRA_OPTIONS)

macro(ADD_SIP_PYTHON_MODULE MODULE_NAME MODULE_SIP)

    set(EXTRA_LINK_LIBRARIES ${ARGN})

    string(REPLACE "." "/" _x ${MODULE_NAME})
    get_filename_component(_parent_module_path ${_x} PATH)
    get_filename_component(_child_module_name ${_x} NAME)

    get_filename_component(_module_path ${MODULE_SIP} PATH)
    get_filename_component(_abs_module_sip ${MODULE_SIP} ABSOLUTE)

    # We give this target a long logical target name.
    # (This is to avoid having the library name clash with any already
    # install library names. If that happens then cmake dependency
    # tracking get confused.)
    string(REPLACE "." "_" _logical_name ${MODULE_NAME})
    set(_logical_name "python_module_${_logical_name}")

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_module_path})    # Output goes in this dir.

    set(_sip_includes)
    foreach(_inc ${SIP_INCLUDES})
        get_filename_component(_abs_inc ${_inc} ABSOLUTE)
        list(APPEND _sip_includes -I ${_abs_inc})
    endforeach()

    set(_sip_tags)
    foreach(_tag ${SIP_TAGS})
        list(APPEND _sip_tags -t ${_tag})
    endforeach()

    set(_sip_x)
    foreach(_x ${SIP_DISABLE_FEATURES})
        list(APPEND _sip_x -x ${_x})
    endforeach()

    set(_message "-DMESSAGE=Generating CPP code for module ${MODULE_NAME}")
    set(_sip_output_files)
    foreach(CONCAT_NUM RANGE 0 ${SIP_CONCAT_PARTS})
        if( ${CONCAT_NUM} LESS ${SIP_CONCAT_PARTS} )
            set(_sip_output_files ${_sip_output_files} ${CMAKE_CURRENT_BINARY_DIR}/${_module_path}/sip${_child_module_name}part${CONCAT_NUM}.cpp )
        endif()
    endforeach()

    # Suppress warnings
    if(PEDANTIC)
      if(MSVC)
        # 4996 deprecation warnings (bindings re-export deprecated methods)
        # 4701 potentially uninitialized variable used (sip generated code)
        # 4702 unreachable code (sip generated code)
        add_definitions( /wd4996 /wd4701 /wd4702 )
      else()
        # disable all warnings
        add_definitions( -w -Wno-deprecated-declarations )
        if(NOT APPLE)
          add_definitions( -fpermissive )
        endif()
      endif()
    endif()


    set(SIPCMD ${SIP_BINARY_PATH} ${_sip_tags} -w -e ${_sip_x} ${SIP_EXTRA_OPTIONS} -j ${SIP_CONCAT_PARTS} -c ${CMAKE_CURRENT_BINARY_DIR}/${_module_path} ${_sip_includes} ${_abs_module_sip})

    add_custom_command(
        OUTPUT ${_sip_output_files}
        COMMAND ${CMAKE_COMMAND} -E echo ${message}
        COMMAND ${CMAKE_COMMAND} -E touch ${_sip_output_files}
        COMMAND ${SIPCMD}
        DEPENDS ${_abs_module_sip} ${SIP_EXTRA_FILES_DEPEND}
        VERBATIM
    )
    # not sure if type MODULE could be uses anywhere, limit to cygwin for now
    if(CYGWIN OR APPLE)
        add_library(${_logical_name} MODULE ${_sip_output_files})
    else()
        add_library(${_logical_name} SHARED ${_sip_output_files})
    endif()
    if(NOT APPLE)
        TARGET_LINK_LIBRARIES(${_logical_name} ${PYTHON_LIBRARY})
    endif()
    target_link_libraries(${_logical_name} ${EXTRA_LINK_LIBRARIES})
    if(APPLE)
        set_target_properties(${_logical_name} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
    endif()
    set_target_properties(${_logical_name} PROPERTIES PREFIX "" OUTPUT_NAME ${_child_module_name})
    set_property(TARGET ${_logical_name} PROPERTY CXX_STANDARD 11)
    if(OSX_FRAMEWORK)
        set_target_properties(${_logical_name} PROPERTIES
            INSTALL_RPATH "@loader_path/../../../../Frameworks/"
        )
    endif()

    if(WIN32)
      set_target_properties(${_logical_name} PROPERTIES COMPILE_FLAGS "/bigobj")
      set_target_properties(${_logical_name} PROPERTIES SUFFIX ".pyd")
    endif()

    if(WIN32)
      get_target_property(_runtime_output ${_logical_name} RUNTIME_OUTPUT_DIRECTORY)
      add_custom_command(TARGET ${_logical_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying extension ${_child_module_name}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${_logical_name}>" "${_runtime_output}/${_child_module_name}.pyd"
        DEPENDS ${_logical_name}
      )
    endif()

    install(TARGETS ${_logical_name} DESTINATION "${NGSTD_PYTHON_DIR}") # /${_parent_module_path}

endmacro()
