
#include "window_tab.h"
#include "imgui.h"
#include "panels/panel_manager.h"

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

            ImGui::EndMenu();
        }
    }
}
