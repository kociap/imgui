cmake_minimum_required(VERSION 3.15)
include(FetchContent)

project(imgui)

add_library(imgui STATIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_demo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui.cpp"
)
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(imgui PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)