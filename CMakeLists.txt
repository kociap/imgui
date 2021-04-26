cmake_minimum_required(VERSION 3.15)
include(FetchContent)

project(imgui)

FetchContent_Declare(
    anton_core
    GIT_REPOSITORY https://github.com/kociap/anton_core.git
)
FetchContent_MakeAvailable(anton_core)

set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_PNG TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_BZip2 TRUE)
set(SKIP_INSTALL_ALL TRUE)

FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/kociap/FreeType.git
)
FetchContent_MakeAvailable(freetype)

add_library(imgui STATIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/private/utility.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/private/utility.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/private/widgets/button.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/private/widgets/checkbox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/private/widgets/collapsible_header.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/private/widgets/combo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/private/widgets/drag.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/private/widgets/outliner_tree.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_demo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp"
)
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private")
set_target_properties(imgui PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
target_link_libraries(imgui PUBLIC anton_core PRIVATE freetype)
target_compile_definitions(imgui
    PRIVATE
    # Windows specific defines
    UNICODE
    _UNICODE
    _CRT_SECURE_NO_WARNINGS
    _SILENCE_CLANG_CONCEPTS_MESSAGE
)
