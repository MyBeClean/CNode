
#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GLFW/glfw3.h>
#include <iostream>

inline bool setAppIcon(GLFWwindow* window)
{
    const char* path = R"(D:\CNode\assets\img\Logo.png)";

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* pixels = stbi_load(
        path,
        &width,
        &height,
        &channels,
        4
    );

    if (!pixels)
    {
        std::cerr << "Gagal memuat logo: "
            << stbi_failure_reason() << '\n';
        return false;
    }

    GLFWimage icon;
    icon.width = width;
    icon.height = height;
    icon.pixels = pixels;

    glfwSetWindowIcon(window, 1, &icon);

    stbi_image_free(pixels);

    return true;
}
