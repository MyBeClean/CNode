
#include "scene/window/logo.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

namespace CNode
{
    bool EditorLogo::Apply(GLFWwindow* window)
    {
        if (!window)
            return false;

        int width = 0;
        int height = 0;
        int channels = 0;

        unsigned char* pixels = stbi_load(
            "assets/img/Logo.png",
            &width,
            &height,
            &channels,
            4
        );

        if (!pixels)
        {
            std::cerr
                << "CNode: Failed to load assets/img/Logo.png: "
                << stbi_failure_reason()
                << '\n';

            return false;
        }

        GLFWimage image{};
        image.width = width;
        image.height = height;
        image.pixels = pixels;

        glfwSetWindowIcon(window, 1, &image);

        stbi_image_free(pixels);

        return true;
    }
}
