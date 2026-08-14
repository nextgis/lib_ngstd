include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(NGSTD_WIDGETS_VERSION "1.0.0")

set(NGSTD_WIDGETS_PUBLIC_HEADERS
    include/ngstd/widgets/adapters.h
    include/ngstd/widgets/basic_components.h
    include/ngstd/widgets/button.h
    include/ngstd/widgets/card.h
    include/ngstd/widgets/combo_box.h
    include/ngstd/widgets/components.h
    include/ngstd/widgets/design_tokens.h
    include/ngstd/widgets/disclosure.h
    include/ngstd/widgets/expandable_section.h
    include/ngstd/widgets/icons.h
    include/ngstd/widgets/motion_adapter.h
    include/ngstd/widgets/navigation_components.h
    include/ngstd/widgets/page_background.h
    include/ngstd/widgets/proxy_style.h
    include/ngstd/widgets/theme.h
    include/ngstd/widgets/theme_switch.h
    include/ngstd/widgets/theme_options.h
    include/ngstd/widgets/types.h
    include/ngstd/widgets/version.h
    include/ngstd/widgets/view_components.h
    include/ngstd/widgets/widget_style.h
    include/ngstd/widgets/widgets.h
)

set(NGSTD_WIDGETS_SOURCES
    src/accessibility.cpp
    src/accessibility_p.h
    src/adapters.cpp
    src/basic_components.cpp
    src/button.cpp
    src/button_label.cpp
    src/button_label_p.h
    src/card.cpp
    src/combo_box.cpp
    src/component_utils.cpp
    src/component_utils_p.h
    src/design_tokens.cpp
    src/design_tokens_p.h
    src/generated/token_data_p.h
    src/disclosure.cpp
    src/expandable_section.cpp
    src/icons.cpp
    src/motion_adapter.cpp
    src/motion_controller.cpp
    src/motion_controller_p.h
    src/navigation_components.cpp
    src/page_background.cpp
    src/proxy_style.cpp
    src/resources.cpp
    src/resources_p.h
    src/state_journal.cpp
    src/state_journal_p.h
    src/theme.cpp
    src/theme_manager.cpp
    src/theme_manager_p.h
    src/theme_switch.cpp
    src/theme_options.cpp
    src/version.cpp
    src/view_components.cpp
    src/widget_style.cpp
    src/widget_decorator.cpp
    src/widget_decorator_p.h
)

add_library(
    ngstd_widgets
    ${NGSTD_WIDGETS_PUBLIC_HEADERS}
    ${NGSTD_WIDGETS_SOURCES}
    resources/widgets.qrc
)
add_library(ngstd::widgets ALIAS ngstd_widgets)
add_library(ngstd_widgets_build_options INTERFACE)

if(MSVC)
    target_compile_options(ngstd_widgets_build_options INTERFACE /W4)
    if(NGSTD_WIDGETS_WARNINGS_AS_ERRORS)
        target_compile_options(ngstd_widgets_build_options INTERFACE /WX)
    endif()
else()
    target_compile_options(
        ngstd_widgets_build_options
        INTERFACE
            -Wall
            -Wcast-align
            -Wconversion
            -Wextra
            -Wformat=2
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Woverloaded-virtual
            -Wpedantic
            -Wshadow
    )
    if(NGSTD_WIDGETS_WARNINGS_AS_ERRORS)
        target_compile_options(ngstd_widgets_build_options INTERFACE -Werror)
    endif()
endif()

if(
    NGSTD_WIDGETS_ENABLE_SANITIZERS
    AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"
)
    target_compile_options(
        ngstd_widgets_build_options
        INTERFACE -fno-omit-frame-pointer -fsanitize=address,undefined
    )
    target_link_options(
        ngstd_widgets_build_options
        INTERFACE -fno-omit-frame-pointer -fsanitize=address,undefined
    )
endif()

if(
    NGSTD_WIDGETS_ENABLE_COVERAGE
    AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"
)
    target_compile_options(
        ngstd_widgets_build_options
        INTERFACE --coverage -O0 -g
    )
    target_link_options(ngstd_widgets_build_options INTERFACE --coverage)
endif()

set_target_properties(
    ngstd_widgets
    PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        CXX_EXTENSIONS OFF
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        EXPORT_NAME widgets
        POSITION_INDEPENDENT_CODE ON
        SOVERSION 1
        VERSION ${NGSTD_WIDGETS_VERSION}
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(ngstd_widgets PRIVATE NGSTD_WIDGETS_LIBRARY)
else()
    target_compile_definitions(ngstd_widgets PUBLIC NGSTD_WIDGETS_STATIC)
endif()

target_compile_definitions(
    ngstd_widgets
    PRIVATE
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
)

target_include_directories(
    ngstd_widgets
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(
    ngstd_widgets
    PRIVATE $<BUILD_INTERFACE:ngstd_widgets_build_options>
    PUBLIC
        ${NGSTD_WIDGETS_QT_PACKAGE}::Core
        ${NGSTD_WIDGETS_QT_PACKAGE}::Gui
        ${NGSTD_WIDGETS_QT_PACKAGE}::Svg
        ${NGSTD_WIDGETS_QT_PACKAGE}::Widgets
)

install(
    TARGETS ngstd_widgets
    EXPORT ngstd_widgets_targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
    FILES
        ${NGSTD_SOURCE_DIR}/LICENSE
        ${NGSTD_SOURCE_DIR}/THIRD_PARTY_NOTICES
        ${NGSTD_SOURCE_DIR}/CHANGELOG.md
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/assets-manifest.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/ngstd_widgets
)
install(
    DIRECTORY ${NGSTD_SOURCE_DIR}/docs/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/ngstd_widgets
    FILES_MATCHING PATTERN "*.md"
)
install(
    DIRECTORY ${NGSTD_SOURCE_DIR}/licenses/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/ngstd_widgets/licenses
    FILES_MATCHING PATTERN "*.txt"
)
install(
    DIRECTORY include/ngstd
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

configure_package_config_file(
    ${NGSTD_SOURCE_DIR}/cmake/ngstd_widgets_config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ngstd_widgets-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ngstd_widgets
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ngstd_widgets-config-version.cmake
    VERSION ${NGSTD_WIDGETS_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ngstd_widgets-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ngstd_widgets-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ngstd_widgets
)
install(
    EXPORT ngstd_widgets_targets
    FILE ngstd_widgets_targets.cmake
    NAMESPACE ngstd::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ngstd_widgets
)
