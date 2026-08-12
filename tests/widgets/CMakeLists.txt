if(NGSTD_WIDGETS_QT_PACKAGE STREQUAL "Qt6")
    find_package(Qt6 6.2 REQUIRED COMPONENTS Test Widgets)
else()
    find_package(Qt5 5.15 REQUIRED COMPONENTS Test Widgets)
endif()

function(ngstd_add_widget_test test_name source_file)
    add_executable(${test_name} ${source_file})
    set_target_properties(
        ${test_name}
        PROPERTIES
            AUTOMOC ON
            CXX_EXTENSIONS OFF
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )
    target_compile_definitions(
        ${test_name}
        PRIVATE
            QT_NO_CAST_FROM_ASCII
            QT_NO_CAST_TO_ASCII
    )
    target_link_libraries(
        ${test_name}
        PRIVATE
            ngstd::widgets
            ngstd_widgets_build_options
            ${NGSTD_WIDGETS_QT_PACKAGE}::Test
            ${NGSTD_WIDGETS_QT_PACKAGE}::Widgets
    )
    if(WIN32)
        add_test(
            NAME ${test_name}
            COMMAND
                ${CMAKE_COMMAND} -E env
                "PATH=$<TARGET_FILE_DIR:ngstd_widgets>\;$ENV{PATH}"
                $<TARGET_FILE:${test_name}>
        )
    else()
        add_test(NAME ${test_name} COMMAND ${test_name})
    endif()
    set_tests_properties(
        ${test_name}
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()

ngstd_add_widget_test(
    ngstd_widgets_theme_lifecycle_tests
    test_theme_lifecycle.cpp
)
ngstd_add_widget_test(ngstd_widgets_tokens_tests test_tokens.cpp)
ngstd_add_widget_test(
    ngstd_widgets_standard_controls_tests
    test_standard_controls.cpp
)
ngstd_add_widget_test(ngstd_widgets_components_tests test_components.cpp)
ngstd_add_widget_test(ngstd_widgets_adapters_tests test_adapters.cpp)
ngstd_add_widget_test(ngstd_widgets_resources_tests test_resources.cpp)

if(NGSTD_WIDGETS_QT_PACKAGE STREQUAL "Qt6")
    add_executable(ngstd_widgets_visual_harness visual_harness.cpp)
    set_target_properties(
        ngstd_widgets_visual_harness
        PROPERTIES
            CXX_EXTENSIONS OFF
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )
    target_compile_definitions(
        ngstd_widgets_visual_harness
        PRIVATE
            QT_NO_CAST_FROM_ASCII
            QT_NO_CAST_TO_ASCII
    )
    target_link_libraries(
        ngstd_widgets_visual_harness
        PRIVATE
            ngstd::widgets
            ngstd_widgets_build_options
            ${NGSTD_WIDGETS_QT_PACKAGE}::Widgets
    )

    add_executable(ngstd_widgets_visual_compare visual_compare.cpp)
    set_target_properties(
        ngstd_widgets_visual_compare
        PROPERTIES
            CXX_EXTENSIONS OFF
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )
    target_compile_definitions(
        ngstd_widgets_visual_compare
        PRIVATE
            QT_NO_CAST_FROM_ASCII
            QT_NO_CAST_TO_ASCII
    )
    target_link_libraries(
        ngstd_widgets_visual_compare
        PRIVATE
            ngstd_widgets_build_options
            ${NGSTD_WIDGETS_QT_PACKAGE}::Gui
    )

    set(widget_visual_actual_directory
        "${CMAKE_CURRENT_BINARY_DIR}/canonical-widgets")
    if(WIN32)
        add_test(
            NAME ngstd_widgets_capture_qt6_canonical
            COMMAND
                ${CMAKE_COMMAND} -E env
                "PATH=$<TARGET_FILE_DIR:ngstd_widgets>\;$ENV{PATH}"
                $<TARGET_FILE:ngstd_widgets_visual_harness>
                --output-directory ${widget_visual_actual_directory}
        )
    else()
        add_test(
            NAME ngstd_widgets_capture_qt6_canonical
            COMMAND
                $<TARGET_FILE:ngstd_widgets_visual_harness>
                --output-directory ${widget_visual_actual_directory}
        )
    endif()
    set_tests_properties(
        ngstd_widgets_capture_qt6_canonical
        PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=1"
            LABELS "widgets;visual;canonical"
    )

    set(widget_visual_baselines
        button
        buttons
        loading-button
        line-edit
        native-combo-box
        native-combo-box-popup
        search-field
        spin-box
        double-spin-box
        check-box
        radio-button
        tag
        tab-widget
        notice
        disclosure
        expandable-section
        expandable-section-expanded
        card
        card-button
        card-button-media
        card-button-background
        card-button-selectable
        card-button-data
        card-button-toolbox
        reveal-widget
        page-background
        list-widget
        tree-widget
        list-widget-hover
        tree-widget-hover
        table-widget
        dialog-button-box
        theme-switch
        spinner
        progress-bar
        combo-box
        toast
        theme-switch-pulse
        disclosure-expanded
        expandable-section-hover
        expandable-section-transition
        button-primary-hover
        button-primary-pressed
        button-disabled-hover
        button-icon-pressed
        button-data-ripple
        button-checkable-checked
        button-checkable-pulse
        card-button-pulse
        button-keyboard-focus
        spin-box-hover
        spin-box-compact
        check-box-pulse
        combo-box-popup
        combo-box-popup-hover
        table-widget-action
    )
    foreach(widget_name IN LISTS widget_visual_baselines)
        add_test(
            NAME ngstd_widgets_visual_qt6_canonical_${widget_name}
            COMMAND
                $<TARGET_FILE:ngstd_widgets_visual_compare>
                ${CMAKE_CURRENT_SOURCE_DIR}/baselines/qt6/${widget_name}.png
                ${widget_visual_actual_directory}/${widget_name}.png
                0.04
        )
        set_tests_properties(
            ngstd_widgets_visual_qt6_canonical_${widget_name}
            PROPERTIES
                DEPENDS ngstd_widgets_capture_qt6_canonical
                LABELS "widgets;visual;canonical"
        )
    endforeach()
endif()

if(NOT NGSTD_WIDGETS_ENABLE_SANITIZERS)
    add_test(
        NAME ngstd_widgets_install_consumer
        COMMAND
            ${CMAKE_COMMAND}
            -DNGSTD_BUILD_DIRECTORY=${CMAKE_BINARY_DIR}
            -DNGSTD_SOURCE_DIRECTORY=${CMAKE_CURRENT_SOURCE_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_install_consumer.cmake
    )
endif()

find_package(Python3 3.8 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    set(widget_tool_directory ${NGSTD_SOURCE_DIR}/tools/widgets)
    set(widget_token_file
        ${NGSTD_SOURCE_DIR}/src/widgets/resources/tokens/nextgis-tokens.json)
    add_test(
        NAME ngstd_widgets_style_resources
        COMMAND
            ${Python3_EXECUTABLE}
            ${widget_tool_directory}/build_style_resources.py
            --check
    )

    add_test(
        NAME ngstd_widgets_token_schema
        COMMAND
            ${Python3_EXECUTABLE}
            ${widget_tool_directory}/validate_tokens.py
            ${widget_token_file}
    )

    add_test(
        NAME ngstd_widgets_token_codegen
        COMMAND
            ${Python3_EXECUTABLE}
            ${widget_tool_directory}/check_generated_tokens.py
            ${widget_token_file}
            ${NGSTD_SOURCE_DIR}/src/widgets/src/generated/token_data_p.h
    )

    add_test(
        NAME ngstd_widgets_asset_manifest
        COMMAND
            ${Python3_EXECUTABLE}
            ${widget_tool_directory}/build_asset_manifest.py
            --check
    )

    add_test(
        NAME ngstd_widgets_license_audit
        COMMAND
            ${Python3_EXECUTABLE}
            ${widget_tool_directory}/check_licenses.py
    )

    if(UNIX AND NOT APPLE AND BUILD_SHARED_LIBS)
        add_test(
            NAME ngstd_widgets_public_exports
            COMMAND
                ${Python3_EXECUTABLE}
                ${widget_tool_directory}/check_public_exports.py
                $<TARGET_FILE:ngstd_widgets>
        )
    endif()
endif()
