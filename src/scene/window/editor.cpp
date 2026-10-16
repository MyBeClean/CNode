
#include "scene/window/editor.h"
#include "scene/window/logo.h"
#include "registration/registration.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

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
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
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

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
        {
            ImGui::DestroyContext();
            glfwDestroyWindow(window);
            window = nullptr;
            glfwTerminate();
            return false;
        }

        if (!ImGui_ImplOpenGL3_Init("#version 330"))
        {
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            glfwDestroyWindow(window);
            window = nullptr;
            glfwTerminate();
            return false;
        }

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
    }

    void EditorWindow::Render()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        CNode::DrawRegisteredWindows();

        ImGui::Render();

        int width = 0;
        int height = 0;

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClearColor(0.12f, 0.13f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void EditorWindow::Shutdown()
    {
        if (ImGui::GetCurrentContext())
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }

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
