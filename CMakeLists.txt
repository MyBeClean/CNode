cmake_minimum_required(VERSION 3.24)

cmake_policy(SET CMP0169 OLD)

project(CNode LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

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

find_package(OpenGL REQUIRED)

add_executable(CNode
    main.cpp
    src/scene/window/editor.cpp
    src/scene/window/logo.cpp
)

target_include_directories(CNode PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${stb_SOURCE_DIR}"
)

target_link_libraries(CNode PRIVATE
    glfw
    OpenGL::GL
)

set_property(
    TARGET CNode
    PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)