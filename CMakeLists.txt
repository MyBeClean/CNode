cmake_minimum_required(VERSION 3.24)

cmake_policy(SET CMP0169 OLD)

project(CNode LANGUAGES CXX)

# ============================================================
# C++ CONFIGURATION
# ============================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ============================================================
# GLFW
# ============================================================

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw)

# ============================================================
# STB
# ============================================================

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

FetchContent_GetProperties(stb)

if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
endif()

# ============================================================
# DEAR IMGUI
# ============================================================

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")

if(NOT EXISTS "${IMGUI_DIR}/imgui.cpp")
    message(FATAL_ERROR
        "Dear ImGui not found in external/imgui"
    )
endif()

find_package(OpenGL REQUIRED)

# ============================================================
# AUTOMATIC WINDOW ATTRIBUTE REGISTRY
# ============================================================

# Cari seluruh header attribute secara rekursif.
# Header baru akan terdeteksi ketika CMake melakukan reconfigure.
file(
    GLOB_RECURSE CNODE_WINDOW_ATTRIBUTE_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tab/*_attribute.h"
)

# Folder untuk source yang dihasilkan CMake.
set(CNODE_GENERATED_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

file(MAKE_DIRECTORY "${CNODE_GENERATED_DIR}")

set(CNODE_ATTRIBUTE_REGISTRY_SOURCE
    "${CNODE_GENERATED_DIR}/window_attribute_registry.cpp"
)

# Header registrasi utama.
set(CNODE_ATTRIBUTE_INCLUDES
    "#include \"registration/registration_data.h\"\n"
)

# Sertakan setiap header attribute secara otomatis.
foreach(ATTRIBUTE_HEADER IN LISTS CNODE_WINDOW_ATTRIBUTE_HEADERS)

    file(
        RELATIVE_PATH ATTRIBUTE_RELATIVE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${ATTRIBUTE_HEADER}"
    )

    file(
        TO_CMAKE_PATH
        "${ATTRIBUTE_RELATIVE_PATH}"
        ATTRIBUTE_RELATIVE_PATH
    )

    string(
        APPEND CNODE_ATTRIBUTE_INCLUDES
        "#include \"${ATTRIBUTE_RELATIVE_PATH}\"\n"
    )

endforeach()

# Buat source registry otomatis.
file(
    WRITE "${CNODE_ATTRIBUTE_REGISTRY_SOURCE}"
    "${CNODE_ATTRIBUTE_INCLUDES}\n"
    "namespace CNode {\n"
    "    void LinkWindowAttributeRegistry() {}\n"
    "}\n"
)

list(LENGTH CNODE_WINDOW_ATTRIBUTE_HEADERS CNODE_WINDOW_ATTRIBUTE_COUNT)

message(STATUS
    "CNode: Found ${CNODE_WINDOW_ATTRIBUTE_COUNT} window attribute headers"
)

# ============================================================
# SOURCE FILES
# ============================================================

add_executable(CNode

    # Main
    main.cpp

    # Scene
    src/scene/window/editor.cpp
    src/scene/window/logo.cpp
    src/scene/window/font.cpp

    # Registration
    src/registration/registration.cpp
    src/registration/registration_data.cpp
    
    src/registration/window_docking.cpp

    # Windows
    src/tab/windows/scene_editor/scene_editor.cpp
    src/tab/windows/inspector/inspector.cpp

    # Automatically generated attribute registry
    "${CNODE_ATTRIBUTE_REGISTRY_SOURCE}"

    # Dear ImGui core
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"

    # Dear ImGui backends
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
 "include/scene/window/font.h" "src/scene/window/font.cpp")

# ============================================================
# INCLUDE DIRECTORIES
# ============================================================

target_include_directories(CNode PRIVATE

    "${CMAKE_CURRENT_SOURCE_DIR}/include"

    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui"

    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends"

    "${stb_SOURCE_DIR}"
)

# ============================================================
# LINK LIBRARIES
# ============================================================

target_link_libraries(CNode PRIVATE

    glfw

    OpenGL::GL
)

# ============================================================
# VISUAL STUDIO DEBUGGING
# ============================================================

set_property(
    TARGET CNode
    PROPERTY VS_DEBUGGER_WORKING_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}"
)