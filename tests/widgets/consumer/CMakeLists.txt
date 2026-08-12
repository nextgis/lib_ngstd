cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(ngstd_widgets_consumer LANGUAGES CXX)

find_package(ngstd_widgets CONFIG REQUIRED)

add_executable(ngstd_widgets_consumer main.cpp)
set_target_properties(
    ngstd_widgets_consumer
    PROPERTIES
        CXX_EXTENSIONS OFF
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)
target_link_libraries(ngstd_widgets_consumer PRIVATE ngstd::widgets)
