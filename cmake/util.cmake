function(ngstd_read_version version_header version_var major_var minor_var patch_var)
    file(READ "${version_header}" version_content)

    if(NOT version_content MATCHES
        "#define[ \t]+NGLIB_MAJOR_VERSION[ \t]+([0-9]+)")
        message(FATAL_ERROR
            "Cannot read NGLIB_MAJOR_VERSION from ${version_header}")
    endif()
    set(major_version "${CMAKE_MATCH_1}")

    if(NOT version_content MATCHES
        "#define[ \t]+NGLIB_MINOR_VERSION[ \t]+([0-9]+)")
        message(FATAL_ERROR
            "Cannot read NGLIB_MINOR_VERSION from ${version_header}")
    endif()
    set(minor_version "${CMAKE_MATCH_1}")

    if(NOT version_content MATCHES
        "#define[ \t]+NGLIB_PATCH_NUMBER[ \t]+([0-9]+)")
        message(FATAL_ERROR
            "Cannot read NGLIB_PATCH_NUMBER from ${version_header}")
    endif()
    set(patch_version "${CMAKE_MATCH_1}")

    set(${version_var}
        "${major_version}.${minor_version}.${patch_version}"
        PARENT_SCOPE
    )
    set(${major_var} "${major_version}" PARENT_SCOPE)
    set(${minor_var} "${minor_version}" PARENT_SCOPE)
    set(${patch_var} "${patch_version}" PARENT_SCOPE)
endfunction()

function(ngstd_configure_qt_target target_name)
    target_compile_features(${target_name} PUBLIC cxx_std_17)
    set_target_properties(${target_name} PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        CXX_EXTENSIONS OFF
    )
endfunction()
