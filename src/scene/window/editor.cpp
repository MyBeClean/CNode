#include "scene/window/editor.h"
#include "scene/window/logo.h"

#include <iostream>

namespace CNode
{
    EditorWindow::EditorWindow() = default;

    EditorWindow::~EditorWindow()
    {
        Shutdown();
    }

    bool EditorWindow::Initialize()
    {
        if (initialized)
            return true;

        if (!glfwInit())
        {
            std::cerr << "CNode: Failed to initialize GLFW.\n";
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(
            GLFW_OPENGL_FORWARD_COMPAT,
            GLFW_TRUE
        );
#endif

        window = glfwCreateWindow(
            1280,
            800,
            "CNode",
            nullptr,
            nullptr
        );

        if (!window)
        {
            std::cerr << "CNode: Failed to create window.\n";
            glfwTerminate();
            return false;
        }

        CNode::EditorLogo::Apply(window);

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        initialized = true;

        std::cout << "CNode: Editor initialized.\n";

        return true;
    }

    void EditorWindow::Run()
    {
        if (!initialized || !window)
            return;

        while (!glfwWindowShouldClose(window))
        {
            Update();
            Render();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    void EditorWindow::Update()
    {
        // Input dan pembaruan editor akan ditambahkan di sini.
    }

    void EditorWindow::Render()
    {
        int width = 0;
        int height = 0;

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClearColor(
            0.12f,
            0.13f,
            0.15f,
            1.0f
        );

        glClear(GL_COLOR_BUFFER_BIT);
    }

    void EditorWindow::Shutdown()
    {
        if (window)
        {
            glfwDestroyWindow(window);
            window = nullptr;
        }

        if (initialized)
        {
            glfwTerminate();
            initialized = false;
        }
    }
}
