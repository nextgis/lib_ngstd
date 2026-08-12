find_package(OpenSSL REQUIRED)
if(WITH_SENTRY)
    find_package(sentry CONFIG REQUIRED)
endif()

set(ngstd_framework_public_headers
    application.h
    framework.h
    mainwindow.h
    minisplitter.h
    navigationpane.h
    navigationwidget.h
    style.h
    styledbar.h
    updater.h
)

set(ngstd_framework_access_headers
    access/access.h
    access/signbutton.h
    access/signdialog.h
)

set(ngstd_framework_logger_headers
    logger/baselogger.h
)

set(ngstd_framework_private_headers
    access/authserverchecker.h
    access/signserver.h
    logger.h
    logger/consolelogger.h
    logger/loggerdecorator.h
)

set(ngstd_framework_sources
    application.cpp
    logger.cpp
    mainwindow.cpp
    minisplitter.cpp
    navigationpane.cpp
    navigationwidget.cpp
    style.cpp
    styledbar.cpp
    updater.cpp
    access/access.cpp
    access/authserverchecker.cpp
    access/sign.cpp
    access/signbutton.cpp
    access/signdialog.cpp
    access/signserver.cpp
    logger/baselogger.cpp
    logger/consolelogger.cpp
    logger/loggerdecorator.cpp
)

if(WITH_SENTRY)
    list(APPEND ngstd_framework_sources logger/sentrylogger.cpp)
    list(APPEND ngstd_framework_private_headers logger/sentrylogger.h)
endif()

set(ngstd_framework_ui_files
    access/signdialog.ui
)

set(ngstd_framework_resources
    resources/framework.qrc
)

set(ngstd_framework_translations
    "${NGSTD_SOURCE_DIR}/translations/ngstd_framework_ru.ts"
    "${NGSTD_SOURCE_DIR}/translations/ngstd_framework_en.ts"
)

set(ngstd_framework_qm_files)
foreach(translation_file IN LISTS ngstd_framework_translations)
    if(EXISTS "${translation_file}")
        if(NGSTD_QT_MAJOR STREQUAL "6")
            qt6_add_translation(generated_qm_file "${translation_file}")
        else()
            qt5_add_translation(generated_qm_file "${translation_file}")
        endif()
        list(APPEND ngstd_framework_qm_files "${generated_qm_file}")
    endif()
endforeach()

add_custom_target(ngstd_framework_translations ALL DEPENDS ${ngstd_framework_qm_files})

if(MSVC)
    list(APPEND ngstd_framework_sources
        "${NGSTD_SOURCE_DIR}/src/resources/windows/ngstd_version.rc")
endif()

add_library(ngstd_framework
    ${ngstd_framework_public_headers}
    ${ngstd_framework_access_headers}
    ${ngstd_framework_logger_headers}
    ${ngstd_framework_private_headers}
    ${ngstd_framework_sources}
    ${ngstd_framework_ui_files}
    ${ngstd_framework_resources}
)
add_library(ngstd::framework ALIAS ngstd_framework)

ngstd_configure_qt_target(ngstd_framework)

if(BUILD_SHARED_LIBS)
    set_target_properties(ngstd_framework PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        DEFINE_SYMBOL NGSTD_FRAMEWORK_LIBRARY
        SOVERSION ${NGLIB_MAJOR_VERSION}
        VERSION ${VERSION}
    )
    if(MSVC)
        set_target_properties(ngstd_framework PROPERTIES IMPORT_SUFFIX "_i.lib")
    endif()
else()
    target_compile_definitions(ngstd_framework PUBLIC NGSTD_STATIC)
    set_target_properties(ngstd_framework PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

if(WITH_SENTRY)
    target_compile_definitions(ngstd_framework PRIVATE NGSTD_WITH_SENTRY)
endif()

target_link_libraries(ngstd_framework
    PUBLIC
        ${QT_VERSION_BASE}::Network
        ${QT_VERSION_BASE}::Widgets
        ${QT_VERSION_BASE}::Concurrent
        ngstd::core
    PRIVATE
        OpenSSL::Crypto
)

if(WITH_SENTRY)
    if(TARGET sentry-native::sentry-native)
        set(ngstd_sentry_target sentry-native::sentry-native)
    elseif(TARGET sentry::sentry)
        set(ngstd_sentry_target sentry::sentry)
    else()
        message(FATAL_ERROR "sentry package was found, but no supported target exists")
    endif()

    target_link_libraries(ngstd_framework PRIVATE ${ngstd_sentry_target})
endif()

target_include_directories(ngstd_framework
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${NGSTD_SOURCE_DIR}/src"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/access"
)
