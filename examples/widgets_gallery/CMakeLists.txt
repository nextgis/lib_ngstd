add_executable(
    ngstd_widgets_gallery
    demo_wizard.cpp
    demo_wizard.h
    gallery_window.cpp
    gallery_window.h
    gallery_tokens.cpp
    gallery_tokens.h
    main.cpp
    reference_widgets.cpp
    reference_widgets.h
    gallery.qrc
)

set_target_properties(
    ngstd_widgets_gallery
    PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        CXX_EXTENSIONS OFF
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

target_compile_definitions(
    ngstd_widgets_gallery
    PRIVATE
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
)

target_link_libraries(
    ngstd_widgets_gallery
    PRIVATE
        ngstd::widgets
        ngstd_widgets_build_options
        ${NGSTD_WIDGETS_QT_PACKAGE}::Widgets
)

if(BUILD_TESTING AND NGSTD_WIDGETS_QT_PACKAGE STREQUAL "Qt6")
    add_executable(
        ngstd_widgets_gallery_visual_compare
        ${NGSTD_SOURCE_DIR}/tests/widgets/visual_compare.cpp
    )
    set_target_properties(
        ngstd_widgets_gallery_visual_compare
        PROPERTIES
            CXX_EXTENSIONS OFF
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )
    target_compile_definitions(
        ngstd_widgets_gallery_visual_compare
        PRIVATE
            QT_NO_CAST_FROM_ASCII
            QT_NO_CAST_TO_ASCII
    )
    target_link_libraries(
        ngstd_widgets_gallery_visual_compare
        PRIVATE
            ngstd_widgets_build_options
            ${NGSTD_WIDGETS_QT_PACKAGE}::Gui
    )

    set(gallery_visual_actual
        "${CMAKE_CURRENT_BINARY_DIR}/qt6-components-dark.png")
    add_test(
        NAME ngstd_widgets_gallery_capture_qt6_components_dark
        COMMAND
            $<TARGET_FILE:ngstd_widgets_gallery>
            --theme dark
            --direction ltr
            --section components
            --viewport 1268x1800
            --screenshot ${gallery_visual_actual}
    )
    set_tests_properties(
        ngstd_widgets_gallery_capture_qt6_components_dark
        PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=1"
            LABELS "gallery;visual"
    )
    add_test(
        NAME ngstd_widgets_gallery_visual_qt6_components_dark
        COMMAND
            $<TARGET_FILE:ngstd_widgets_gallery_visual_compare>
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/baselines/qt6-components-dark.png
            ${gallery_visual_actual}
            0.04
    )
    set_tests_properties(
        ngstd_widgets_gallery_visual_qt6_components_dark
        PROPERTIES
            DEPENDS ngstd_widgets_gallery_capture_qt6_components_dark
            LABELS "gallery;visual"
    )

    set(gallery_startup_actual
        "${CMAKE_CURRENT_BINARY_DIR}/qt6-components-dark-startup.png")
    add_test(
        NAME ngstd_widgets_gallery_capture_qt6_startup
        COMMAND
            $<TARGET_FILE:ngstd_widgets_gallery>
            --theme dark
            --direction ltr
            --section components
            --viewport 1268x1800
            --screenshot-delay 0
            --screenshot ${gallery_startup_actual}
    )
    set_tests_properties(
        ngstd_widgets_gallery_capture_qt6_startup
        PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=1"
            LABELS "gallery;startup;visual"
    )
    add_test(
        NAME ngstd_widgets_gallery_visual_qt6_startup
        COMMAND
            $<TARGET_FILE:ngstd_widgets_gallery_visual_compare>
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/baselines/qt6-components-dark.png
            ${gallery_startup_actual}
            0.04
    )
    set_tests_properties(
        ngstd_widgets_gallery_visual_qt6_startup
        PROPERTIES
            DEPENDS ngstd_widgets_gallery_capture_qt6_startup
            LABELS "gallery;startup;visual"
    )

    function(ngstd_add_gallery_section_visual section_name viewport_size)
        set(section_actual
            "${CMAKE_CURRENT_BINARY_DIR}/qt6-${section_name}-dark.png")
        set(section_baseline
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/baselines/qt6-${section_name}-dark.png")
        set(section_capture_test
            "ngstd_widgets_gallery_capture_qt6_${section_name}_dark")
        add_test(
            NAME ${section_capture_test}
            COMMAND
                $<TARGET_FILE:ngstd_widgets_gallery>
                --theme dark
                --direction ltr
                --section ${section_name}
                --viewport ${viewport_size}
                --screenshot ${section_actual}
        )
        set_tests_properties(
            ${section_capture_test}
            PROPERTIES
                ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=1"
                LABELS "gallery;visual"
        )
        set(section_compare_test
            "ngstd_widgets_gallery_visual_qt6_${section_name}_dark")
        add_test(
            NAME ${section_compare_test}
            COMMAND
                $<TARGET_FILE:ngstd_widgets_gallery_visual_compare>
                ${section_baseline}
                ${section_actual}
                0.04
        )
        set_tests_properties(
            ${section_compare_test}
            PROPERTIES
                DEPENDS ${section_capture_test}
                LABELS "gallery;visual"
        )

        set(section_cycle_actual
            "${CMAKE_CURRENT_BINARY_DIR}/qt6-${section_name}-dark-after-theme-cycle.png")
        set(section_cycle_test
            "ngstd_widgets_gallery_theme_cycle_qt6_${section_name}")
        add_test(
            NAME ${section_cycle_test}
            COMMAND
                $<TARGET_FILE:ngstd_widgets_gallery>
                --theme dark
                --direction ltr
                --section ${section_name}
                --viewport ${viewport_size}
                --theme-cycle-probe
                --screenshot ${section_cycle_actual}
        )
        set_tests_properties(
            ${section_cycle_test}
            PROPERTIES
                ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=1"
                LABELS "gallery;theme;visual"
                TIMEOUT 60
        )
        set(section_cycle_compare_test
            "ngstd_widgets_gallery_visual_qt6_${section_name}_theme_cycle")
        add_test(
            NAME ${section_cycle_compare_test}
            COMMAND
                $<TARGET_FILE:ngstd_widgets_gallery_visual_compare>
                ${section_actual}
                ${section_cycle_actual}
                0
        )
        set_tests_properties(
            ${section_cycle_compare_test}
            PROPERTIES
                DEPENDS "${section_capture_test};${section_cycle_test}"
                LABELS "gallery;theme;visual"
        )
    endfunction()

    ngstd_add_gallery_section_visual(backgrounds 1268x1800)
    ngstd_add_gallery_section_visual(brand 1268x1600)
    ngstd_add_gallery_section_visual(icons 1268x900)

endif()

if(BUILD_TESTING)
    set(gallery_theme_cycle_test
        "ngstd_widgets_gallery_theme_cycle_qt${NGSTD_QT_MAJOR}")
    set(gallery_theme_cycle_actual
        "${CMAKE_CURRENT_BINARY_DIR}/qt${NGSTD_QT_MAJOR}-components-dark-after-theme-cycle.png")
    add_test(
        NAME ${gallery_theme_cycle_test}
        COMMAND
            $<TARGET_FILE:ngstd_widgets_gallery>
            --theme dark
            --section components
            --viewport 1268x1800
            --screenshot ${gallery_theme_cycle_actual}
            --theme-cycle-probe
    )
    set_tests_properties(
        ${gallery_theme_cycle_test}
        PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=1"
            LABELS "gallery;theme"
            TIMEOUT 60
    )

    set(gallery_adapter_probe_directory
        "${CMAKE_CURRENT_BINARY_DIR}/adapter-probe-qt${NGSTD_QT_MAJOR}")
    add_test(
        NAME ngstd_widgets_gallery_adapter_probe_qt${NGSTD_QT_MAJOR}
        COMMAND
            $<TARGET_FILE:ngstd_widgets_gallery>
            --theme dark
            --adapter-probe ${gallery_adapter_probe_directory}
    )
    set_tests_properties(
        ngstd_widgets_gallery_adapter_probe_qt${NGSTD_QT_MAJOR}
        PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=1"
            LABELS "adapters;gallery"
            TIMEOUT 15
    )

    if(NGSTD_WIDGETS_QT_PACKAGE STREQUAL "Qt6")
        set(gallery_interaction_probe_directory
            "${CMAKE_CURRENT_BINARY_DIR}/interaction-probe-qt6")
        add_test(
            NAME ngstd_widgets_gallery_interaction_probe_qt6
            COMMAND
                $<TARGET_FILE:ngstd_widgets_gallery>
                --theme dark
                --section components
                --viewport 1268x900
                --interaction-probe ${gallery_interaction_probe_directory}
        )
        set_tests_properties(
            ngstd_widgets_gallery_interaction_probe_qt6
            PROPERTIES
                ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=1"
                LABELS "gallery;interaction;visual"
                TIMEOUT 15
        )
        foreach(interaction_name IN ITEMS
                source-combo-popup-hover)
            add_test(
                NAME ngstd_widgets_gallery_visual_qt6_${interaction_name}
                COMMAND
                    $<TARGET_FILE:ngstd_widgets_gallery_visual_compare>
                    ${CMAKE_CURRENT_SOURCE_DIR}/tests/baselines/qt6-${interaction_name}-dark.png
                    ${gallery_interaction_probe_directory}/${interaction_name}.png
                    0.04
            )
            set_tests_properties(
                ngstd_widgets_gallery_visual_qt6_${interaction_name}
                PROPERTIES
                    DEPENDS ngstd_widgets_gallery_interaction_probe_qt6
                    LABELS "gallery;interaction;visual"
            )
        endforeach()
    endif()

    if(NGSTD_WIDGETS_QT_PACKAGE STREQUAL "Qt6")
        add_test(
            NAME ngstd_widgets_gallery_visual_qt6_theme_cycle_stability
            COMMAND
                $<TARGET_FILE:ngstd_widgets_gallery_visual_compare>
                ${gallery_visual_actual}
                ${gallery_theme_cycle_actual}
                0
        )
        set_tests_properties(
            ngstd_widgets_gallery_visual_qt6_theme_cycle_stability
            PROPERTIES
                DEPENDS
                    "ngstd_widgets_gallery_capture_qt6_components_dark;${gallery_theme_cycle_test}"
                LABELS "gallery;theme;visual"
        )
    endif()
endif()
