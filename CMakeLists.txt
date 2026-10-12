cmake_minimum_required(VERSION 3.16)
project(csopesy_os LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

set(SRC_DIR   "${CMAKE_SOURCE_DIR}/CSOPESY")
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third_party/imgui")
set(GLFW_DIR  "${CMAKE_SOURCE_DIR}/third_party/glfw")

add_executable(csopesy_os
    "${SRC_DIR}/main.cpp"
    "${SRC_DIR}/Config.cpp"
    "${SRC_DIR}/Config.h"
    "${SRC_DIR}/AppState.h"
    "${SRC_DIR}/Desktop.cpp"
    "${SRC_DIR}/Desktop.h"
    "${SRC_DIR}/Taskbar.cpp"
    "${SRC_DIR}/Taskbar.h"
    "${SRC_DIR}/TaskManager.cpp"
    "${SRC_DIR}/TaskManager.h"
    "${SRC_DIR}/SystemInfoWindow.cpp"
    "${SRC_DIR}/SystemInfoWindow.h"
    "${SRC_DIR}/FileExplorerWindow.cpp"
    "${SRC_DIR}/FileExplorerWindow.h"
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

target_include_directories(csopesy_os PRIVATE
    "${SRC_DIR}"
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
    "${GLFW_DIR}/include"
)

if(MSVC)
    target_compile_options(csopesy_os PRIVATE /utf-8)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(GLFW_LIB "${GLFW_DIR}/lib-vc2010-64/glfw3.lib")
    else()
        set(GLFW_LIB "${GLFW_DIR}/lib-vc2010-32/glfw3.lib")
    endif()
    target_link_libraries(csopesy_os PRIVATE
        OpenGL::GL
        "${GLFW_LIB}"
        gdi32
        shell32
        user32
        legacy_stdio_definitions
    )
    target_link_options(csopesy_os PRIVATE $<$<CONFIG:Debug>:/NODEFAULTLIB:msvcrt.lib>)
else()
    # Non-MSVC fallback (MinGW / Linux requires system/MSYS2 glfw3 package installed)
    find_package(glfw3 REQUIRED)
    target_link_libraries(csopesy_os PRIVATE OpenGL::GL glfw)
    if(WIN32)
        target_link_libraries(csopesy_os PRIVATE gdi32 shell32 user32)
    endif()
endif()

add_custom_command(TARGET csopesy_os POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SRC_DIR}/config.txt" "$<TARGET_FILE_DIR:csopesy_os>/config.txt"
)
