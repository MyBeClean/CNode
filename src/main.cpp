
#include <filesystem>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "app_icon.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "panels/panel_manager.h"
#include "tab/file/file_tab.h"
#include "tab/windows/window_tab.h"
#include "tab/windows/scene_editor/scene_editor.h"
#include "tab/windows/scene_editor/scene_viewport_renderer.h"

#include "class/register_classes.h"
#include "node/3d_object/cube/cube_render.h"

int main()
{
    namespace fs = std::filesystem;

    fs::path projectRoot = fs::current_path();

    while (!fs::exists(projectRoot / "CMakeLists.txt"))
    {
        const fs::path parent = projectRoot.parent_path();

        if (parent == projectRoot || parent.empty())
            return -1;

        projectRoot = parent;
    }

    const fs::path assetsPath = projectRoot / "assets";
    const fs::path fontPath = assetsPath / "fonts" / "Inter.ttf";
    const fs::path iconPath = assetsPath / "img" / "Logo.png";
    const fs::path layoutPath = projectRoot / "imgui.ini";

    const bool hasSavedLayout = fs::exists(layoutPath);

    const std::string fontFile = fontPath.string();
    const std::string iconFile = iconPath.string();
    const std::string layoutFile = layoutPath.string();

    // GLFW
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        1280, 720, "CNode Engine", nullptr, nullptr
    );

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // GLAD: harus dilakukan setelah context aktif.
    if (!gladLoadGLLoader(
        reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    setAppIcon(window, iconFile.c_str());

    glfwSwapInterval(1);

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = layoutFile.c_str();

    ImFont* interFont = io.Fonts->AddFontFromFileTTF(
        fontFile.c_str(), 16.0f
    );

    if (!interFont)
    {
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Register classes.
    if (!CNode::RegisterAllClasses())
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Cube renderer.
    if (!CNode::CubeRenderer::Get().Initialize())
    {
        CNode::CubeRenderer::Get().Shutdown();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Viewport renderer.
    if (!CNode::SceneViewportRenderer::Get().Initialize())
    {
        CNode::SceneViewportRenderer::Get().Shutdown();
        CNode::CubeRenderer::Get().Shutdown();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Default panels.
    CNode::AddPanel(CNode::PanelType::SceneEditor, false);
    CNode::AddPanel(CNode::PanelType::NodeGraphEditor, false);
    CNode::AddPanel(CNode::PanelType::Inspector, false);

    bool layoutInitialized = false;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGuiID dockspaceID = ImGui::GetID("CNodeDockspace");

        ImGui::DockSpaceOverViewport(dockspaceID, viewport);

        if (!layoutInitialized && !hasSavedLayout)
        {
            ImGui::DockBuilderRemoveNode(dockspaceID);
            ImGui::DockBuilderAddNode(
                dockspaceID, ImGuiDockNodeFlags_DockSpace
            );
            ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

            ImGuiID leftID = 0;
            ImGuiID inspectorID = 0;

            ImGui::DockBuilderSplitNode(
                dockspaceID, ImGuiDir_Right, 0.25f,
                &inspectorID, &leftID
            );

            ImGuiID sceneID = 0;
            ImGuiID graphID = 0;

            ImGui::DockBuilderSplitNode(
                leftID, ImGuiDir_Down, 0.40f,
                &graphID, &sceneID
            );

            ImGui::DockBuilderDockWindow(
                "Scene Editor 1###CNodePanel_1", sceneID
            );
            ImGui::DockBuilderDockWindow(
                "Node Graph Editor 1###CNodePanel_2", graphID
            );
            ImGui::DockBuilderDockWindow(
                "Inspector 1###CNodePanel_3", inspectorID
            );

            ImGui::DockBuilderFinish(dockspaceID);
            layoutInitialized = true;
        }
        else if (hasSavedLayout)
        {
            layoutInitialized = true;
        }

        if (ImGui::BeginMainMenuBar())
        {
            CNode::DrawFileTab();
            CNode::DrawWindowTab();
            ImGui::EndMainMenuBar();
        }

        CNode::DrawAllPanels();

        ImGui::Render();

        int displayWidth = 0;
        int displayHeight = 0;

        glfwGetFramebufferSize(
            window, &displayWidth, &displayHeight
        );

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, displayWidth, displayHeight);

        glClearColor(0.08f, 0.08f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Lepaskan resource saat context masih hidup.
    CNode::SceneViewportRenderer::Get().Shutdown();
    CNode::CubeRenderer::Get().Shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
