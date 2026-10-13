cmake_minimum_required(VERSION 3.20)

project(CNode LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")

add_executable(CNodeEditor
    src/main.cpp

    src/tab/windows/scene_editor/scene_editor.cpp
    src/tab/windows/node_graph/node_graph_editor.cpp
    src/tab/windows/inspector/inspector.cpp
    src/tab/windows/window_tab.cpp

    src/tab/file/file_tab.cpp
    src/tab/file/new_scene.cpp
    src/tab/file/open_scene.cpp

    src/panels/panel_manager.cpp

    src/environment/environment_settings.cpp


    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"

    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
 "src/environment/environment_settings.h" "src/environment/environment_settings.cpp" "src/tab/windows/project_setting/project_setting.h" "src/tab/windows/project_setting/project_setting.cpp")

target_include_directories(CNodeEditor PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
)

target_link_libraries(CNodeEditor PRIVATE
    glfw
    OpenGL::GL
)