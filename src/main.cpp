
#include <iostream>
#include <GLFW/glfw3.h>
#include "app_icon.h"

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Gagal menginisialisasi GLFW!\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        "CNode Engine",
        nullptr,
        nullptr
    );

    if (!window)
    {
        std::cerr << "Gagal membuat jendela!\n";
        glfwTerminate();
        return -1;
    }

    setAppIcon(window);

    glfwMakeContextCurrent(window);

    std::cout << "CNode Engine berhasil dibuka!\n";

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
