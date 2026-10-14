
#include "panel_manager.h"

#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"

#include "tab/windows/scene_editor/scene_editor.h"
#include "tab/windows/node_graph/node_graph_editor.h"
#include "tab/windows/inspector/inspector.h"
#include "tab/windows/project_setting/project_setting.h"
#include "tab/windows/object_list/object_list.h"

namespace CNode
{
    struct Panel
    {
        int id;
        int instance;
        PanelType type;
        bool open;
        bool floatOnFirstFrame;
    };

    static std::vector<Panel> panels;

    static int nextPanelId = 1;

    static int sceneEditorCount = 0;
    static int nodeGraphEditorCount = 0;
    static int inspectorCount = 0;
    static int projectSettingsCount = 0;
    static int objectListCount = 0;

    static std::string GetPanelTitle(const Panel& panel)
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

        case PanelType::ProjectSettings:
            name = "Project Settings";
            break;

        case PanelType::ObjectList:
            name = "Object List";
            break;
        }

        return name + " " + std::to_string(panel.instance) +
            "###CNodePanel_" + std::to_string(panel.id);
    }

    void AddPanel(PanelType type, bool floating)
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

        case PanelType::ProjectSettings:
            instance = ++projectSettingsCount;
            break;

        case PanelType::ObjectList:
            instance = ++objectListCount;
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

    static void DrawPanel(Panel& panel)
    {
        if (!panel.open)
            return;

        const std::string title = GetPanelTitle(panel);

        if (panel.floatOnFirstFrame)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();

            const float offset =
                static_cast<float>((panel.id % 6) * 28);

            ImGui::SetNextWindowPos(
                ImVec2(
                    viewport->WorkPos.x + 80.0f + offset,
                    viewport->WorkPos.y + 60.0f + offset
                ),
                ImGuiCond_Appearing
            );

            ImGui::SetNextWindowSize(
                ImVec2(760.0f, 540.0f),
                ImGuiCond_Appearing
            );

            ImGui::SetNextWindowDockID(
                0,
                ImGuiCond_Appearing
            );

            panel.floatOnFirstFrame = false;
        }

        switch (panel.type)
        {
        case PanelType::SceneEditor:
            DrawSceneEditor(title.c_str(), &panel.open);
            break;

        case PanelType::NodeGraphEditor:
            DrawNodeGraphEditor(title.c_str(), &panel.open);
            break;

        case PanelType::Inspector:
            DrawInspector(title.c_str(), &panel.open);
            break;

        case PanelType::ProjectSettings:
            DrawProjectSettings(title.c_str(), &panel.open);
            break;

        case PanelType::ObjectList:
            DrawObjectList(title.c_str(), &panel.open);
            break;
        }
    }

    void DrawAllPanels()
    {
        for (Panel& panel : panels)
        {
            DrawPanel(panel);
        }
    }
}
