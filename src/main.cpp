
#include <filesystem>

#include <GLFW/glfw3.h>

#include "app_icon.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "panels/panel_manager.h"
#include "tab/file/file_tab.h"
#include "tab/windows/window_tab.h"

int main()
{
    // Memeriksa apakah layout sebelumnya sudah tersimpan.
    const char* layoutFile = "imgui.ini";
    const bool hasSavedLayout = std::filesystem::exists(layoutFile);

    // Inisialisasi GLFW.
    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        "CNode Engine",
        nullptr,
        nullptr
    );

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    setAppIcon(window);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Inisialisasi Dear ImGui.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = layoutFile;

    // Memuat font Inter.
    ImFont* interFont = io.Fonts->AddFontFromFileTTF(
        R"(D:\CNode\assets\fonts\Inter.ttf)",
        16.0f
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
    ImGui_ImplOpenGL3_Init("#version 130");

    // Membuat panel default.
    CNode::AddPanel(CNode::PanelType::SceneEditor, false);
    CNode::AddPanel(CNode::PanelType::NodeGraphEditor, false);
    CNode::AddPanel(CNode::PanelType::Inspector, false);

    bool layoutInitialized = false;

    // Main loop.
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Dockspace utama.
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGuiID dockspaceID = ImGui::GetID("CNodeDockspace");

        ImGui::DockSpaceOverViewport(
            dockspaceID,
            viewport
        );

        // Membuat layout default jika belum ada layout tersimpan.
        if (!layoutInitialized && !hasSavedLayout)
        {
            ImGui::DockBuilderRemoveNode(dockspaceID);

            ImGui::DockBuilderAddNode(
                dockspaceID,
                ImGuiDockNodeFlags_DockSpace
            );

            ImGui::DockBuilderSetNodeSize(
                dockspaceID,
                viewport->Size
            );

            // Membagi area kanan untuk Inspector.
            ImGuiID leftID = 0;
            ImGuiID inspectorID = 0;

            ImGui::DockBuilderSplitNode(
                dockspaceID,
                ImGuiDir_Right,
                0.25f,
                &inspectorID,
                &leftID
            );

            // Membagi area kiri menjadi Scene Editor dan Node Graph.
            ImGuiID sceneID = 0;
            ImGuiID graphID = 0;

            ImGui::DockBuilderSplitNode(
                leftID,
                ImGuiDir_Down,
                0.40f,
                &graphID,
                &sceneID
            );

            // Menempatkan panel ke area masing-masing.
            ImGui::DockBuilderDockWindow(
                "Scene Editor 1###CNodePanel_1",
                sceneID
            );

            ImGui::DockBuilderDockWindow(
                "Node Graph Editor 1###CNodePanel_2",
                graphID
            );

            ImGui::DockBuilderDockWindow(
                "Inspector 1###CNodePanel_3",
                inspectorID
            );

            ImGui::DockBuilderFinish(dockspaceID);

            layoutInitialized = true;
        }
        else if (hasSavedLayout)
        {
            // ImGui akan menggunakan layout dari imgui.ini.
            layoutInitialized = true;
        }

        // Menu utama.
        if (ImGui::BeginMainMenuBar())
        {
            CNode::DrawFileTab();
            CNode::DrawWindowTab();

            ImGui::EndMainMenuBar();
        }

        // Menggambar semua panel melalui panel manager.
        CNode::DrawAllPanels();

        // Rendering.
        ImGui::Render();

        int displayWidth = 0;
        int displayHeight = 0;

        glfwGetFramebufferSize(
            window,
            &displayWidth,
            &displayHeight
        );

        glViewport(0, 0, displayWidth, displayHeight);

        glClearColor(
            0.08f,
            0.08f,
            0.09f,
            1.0f
        );

        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(
            ImGui::GetDrawData()
        );

        glfwSwapBuffers(window);
    }

    // Membersihkan resource.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
