set(ngstd_core_public_headers
    application.h
    core.h
    request.h
    util.h
    version.h
)

set(ngstd_core_sources
    application.cpp
    core.cpp
    request.cpp
    util.cpp
)

set(ngstd_core_translations
    "${NGSTD_SOURCE_DIR}/translations/ngstd_core_ru.ts"
    "${NGSTD_SOURCE_DIR}/translations/ngstd_core_en.ts"
)

set(ngstd_core_qm_files)
foreach(translation_file IN LISTS ngstd_core_translations)
    if(EXISTS "${translation_file}")
        if(NGSTD_QT_MAJOR STREQUAL "6")
            qt6_add_translation(generated_qm_file "${translation_file}")
        else()
            qt5_add_translation(generated_qm_file "${translation_file}")
        endif()
        list(APPEND ngstd_core_qm_files "${generated_qm_file}")
    endif()
endforeach()

add_custom_target(ngstd_core_translations ALL DEPENDS ${ngstd_core_qm_files})

if(MSVC)
    list(APPEND ngstd_core_sources
        "${NGSTD_SOURCE_DIR}/src/resources/windows/ngstd_version.rc")
endif()

add_library(ngstd_core
    ${ngstd_core_public_headers}
    ${ngstd_core_sources}
)
add_library(ngstd::core ALIAS ngstd_core)

ngstd_configure_qt_target(ngstd_core)

if(BUILD_SHARED_LIBS)
    set_target_properties(ngstd_core PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        DEFINE_SYMBOL NGSTD_CORE_LIBRARY
        SOVERSION ${NGLIB_MAJOR_VERSION}
        VERSION ${VERSION}
    )
    if(MSVC)
        set_target_properties(ngstd_core PROPERTIES IMPORT_SUFFIX "_i.lib")
    endif()
else()
    target_compile_definitions(ngstd_core PUBLIC NGSTD_STATIC)
    set_target_properties(ngstd_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_compile_definitions(ngstd_core
    PRIVATE
        NGSTD_PLUGIN_LIB_DIR="${NGSTD_PLUGIN_LIB_DIR}"
)

target_link_libraries(ngstd_core
    PUBLIC
        ${QT_VERSION_BASE}::Core
        ${QT_VERSION_BASE}::Network
)

target_include_directories(ngstd_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${NGSTD_SOURCE_DIR}/src"
)
