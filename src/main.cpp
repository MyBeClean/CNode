
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include <GLFW/glfw3.h>
#include "app_icon.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "tab/windows/scene_editor.h"
#include "tab/windows/node_graph_editor.h"
#include "tab/windows/inspector.h"

enum class PanelType
{
    SceneEditor,
    NodeGraphEditor,
    Inspector
};

struct Panel
{
    int id;
    int instance;
    PanelType type;
    bool open;
    bool floatOnFirstFrame;
};

std::vector<Panel> panels;

int nextPanelId = 1;
int sceneEditorCount = 0;
int nodeGraphEditorCount = 0;
int inspectorCount = 0;

void AddPanel(PanelType type, bool floating = true)
{
    int instance = 0;

    switch (type)
    {
    case PanelType::SceneEditor:
        instance = ++sceneEditorCount;
        break;

    case PanelType::NodeGraphEditor:
        instance = ++nodeGraphEditorCount;
        break;

    case PanelType::Inspector:
        instance = ++inspectorCount;
        break;
    }

    panels.push_back({
        nextPanelId++,
        instance,
        type,
        true,
        floating
        });
}

std::string GetPanelTitle(const Panel& panel)
{
    std::string name;

    switch (panel.type)
    {
    case PanelType::SceneEditor:
        name = "Scene Editor";
        break;

    case PanelType::NodeGraphEditor:
        name = "Node Graph Editor";
        break;

    case PanelType::Inspector:
        name = "Inspector";
        break;
    }

    return name + " " + std::to_string(panel.instance) +
        "###CNodePanel_" + std::to_string(panel.id);
}

void DrawPanel(Panel& panel)
{
    if (!panel.open)
        return;

    std::string title = GetPanelTitle(panel);

    if (panel.floatOnFirstFrame)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        float offset = static_cast<float>((panel.id % 6) * 28);

        ImGui::SetNextWindowPos(
            ImVec2(viewport->WorkPos.x + 80 + offset,
                viewport->WorkPos.y + 60 + offset),
            ImGuiCond_Appearing
        );

        ImGui::SetNextWindowSize(
            ImVec2(760, 540),
            ImGuiCond_Appearing
        );

        ImGui::SetNextWindowDockID(0, ImGuiCond_Appearing);

        panel.floatOnFirstFrame = false;
    }

    switch (panel.type)
    {
    case PanelType::SceneEditor:
        CNode::DrawSceneEditor(title.c_str(), &panel.open);
        break;

    case PanelType::NodeGraphEditor:
        CNode::DrawNodeGraphEditor(title.c_str(), &panel.open);
        break;

    case PanelType::Inspector:
        CNode::DrawInspector(title.c_str(), &panel.open);
        break;
    }
}

int main()
{
    const char* layoutFile = "imgui.ini";
    const bool hasSavedLayout = std::filesystem::exists(layoutFile);

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = layoutFile;

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

    AddPanel(PanelType::SceneEditor, false);
    AddPanel(PanelType::NodeGraphEditor, false);
    AddPanel(PanelType::Inspector, false);

    bool layoutInitialized = false;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGuiID dockspaceID = ImGui::GetID("CNodeDockspace");

        ImGui::DockSpaceOverViewport(
            dockspaceID,
            viewport
        );

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

            ImGuiID leftID = 0;
            ImGuiID inspectorID = 0;

            ImGui::DockBuilderSplitNode(
                dockspaceID,
                ImGuiDir_Right,
                0.25f,
                &inspectorID,
                &leftID
            );

            ImGuiID sceneID = 0;
            ImGuiID graphID = 0;

            ImGui::DockBuilderSplitNode(
                leftID,
                ImGuiDir_Down,
                0.40f,
                &graphID,
                &sceneID
            );

            for (const Panel& panel : panels)
            {
                std::string title = GetPanelTitle(panel);

                switch (panel.type)
                {
                case PanelType::SceneEditor:
                    ImGui::DockBuilderDockWindow(
                        title.c_str(),
                        sceneID
                    );
                    break;

                case PanelType::NodeGraphEditor:
                    ImGui::DockBuilderDockWindow(
                        title.c_str(),
                        graphID
                    );
                    break;

                case PanelType::Inspector:
                    ImGui::DockBuilderDockWindow(
                        title.c_str(),
                        inspectorID
                    );
                    break;
                }
            }

            ImGui::DockBuilderFinish(dockspaceID);
            layoutInitialized = true;
        }
        else if (hasSavedLayout)
        {
            layoutInitialized = true;
        }

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("CNode Engine", nullptr, false, false);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Window"))
            {
                if (ImGui::MenuItem("Scene Editor"))
                    AddPanel(PanelType::SceneEditor);

                if (ImGui::MenuItem("Node Graph Editor"))
                    AddPanel(PanelType::NodeGraphEditor);

                if (ImGui::MenuItem("Inspector"))
                    AddPanel(PanelType::Inspector);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        for (Panel& panel : panels)
            DrawPanel(panel);

        ImGui::Render();

        int displayWidth = 0;
        int displayHeight = 0;

        glfwGetFramebufferSize(
            window,
            &displayWidth,
            &displayHeight
        );

        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.08f, 0.08f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(
            ImGui::GetDrawData()
        );

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
