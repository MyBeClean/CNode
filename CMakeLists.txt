
cmake_minimum_required(VERSION 3.20)

project(CNode LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# =========================================================
# GLFW
# =========================================================

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

# =========================================================
# GLM
# =========================================================

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glm)

# =========================================================
# OPENGL
# =========================================================

find_package(OpenGL REQUIRED)

# =========================================================
# DEPENDENCIES
# =========================================================

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glad")

# =========================================================
# EXECUTABLE
# =========================================================

add_executable(CNodeEditor
    src/main.cpp

    # Panels
    src/panels/panel_manager.cpp

    # Tabs
    src/tab/file/file_tab.cpp
    src/tab/file/new_scene.cpp
    src/tab/file/open_scene.cpp

    src/tab/windows/window_tab.cpp
    src/tab/windows/node_graph/node_graph_editor.cpp
    src/tab/windows/inspector/inspector.cpp
    src/tab/windows/object_list/object_list.cpp

    # Scene editor
    src/tab/windows/scene_editor/scene_editor.cpp
    src/tab/windows/scene_editor/scene_viewport_renderer.cpp

    # Project settings
    src/tab/windows/project_setting/project_setting.cpp
    src/tab/windows/project_setting/environment/environment_settings.cpp

    # Cube
    src/node/3d_object/cube/cube.cpp
    src/node/3d_object/cube/cube_render.cpp
    src/node/3d_object/cube/cube_mapping.cpp
    src/node/3d_object/cube/cube_spawn.cpp

    # Scene objects
    src/scene/scene_objects.cpp

    # Class registration
    src/class/register_classes.cpp
    src/class/transform/transform.cpp
    src/class/visual/visual.cpp

    # GLAD
    "${GLAD_DIR}/src/glad.c"

    # ImGui core
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"

    # ImGui backends
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

# =========================================================
# INCLUDE DIRECTORIES
# =========================================================

target_include_directories(CNodeEditor PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/external"

    "${GLAD_DIR}/include"

    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
)

# =========================================================
# LINK LIBRARIES
# =========================================================

target_link_libraries(CNodeEditor PRIVATE
    glfw
    OpenGL::GL
    glm::glm
)
