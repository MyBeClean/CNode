
#include "window_tab.h"
#include "panels/panel_manager.h"

#include "imgui.h"

namespace CNode
{
    void DrawWindowTab()
    {
        if (ImGui::BeginMenu("Window"))
        {
            if (ImGui::MenuItem("Scene Editor"))
                AddPanel(PanelType::SceneEditor);

            if (ImGui::MenuItem("Node Graph Editor"))
                AddPanel(PanelType::NodeGraphEditor);

            if (ImGui::MenuItem("Inspector"))
                AddPanel(PanelType::Inspector);

            if (ImGui::MenuItem("Project Settings"))
                AddPanel(PanelType::ProjectSettings);

            if (ImGui::MenuItem("Object List"))
            {
                CNode::AddPanel(CNode::PanelType::ObjectList);
            }

            ImGui::EndMenu();
        }
    }
}
