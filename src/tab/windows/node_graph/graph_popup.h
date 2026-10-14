
#pragma once

#include "imgui.h"

#include "tab/windows/node_graph/node_graph_data.h"
#include "node/3d_object/cube/cube.h"

namespace CNode
{
    struct GraphPopupState
    {
        ImVec2 screenPosition = ImVec2(0.0f, 0.0f);
        ImVec2 graphPosition = ImVec2(100.0f, 100.0f);
    };

    inline void OpenGraphPopup(
        GraphPopupState& state,
        const ImVec2& screenPosition,
        const ImVec2& graphPosition)
    {
        state.screenPosition = screenPosition;
        state.graphPosition = graphPosition;

        ImGui::OpenPopup("CNodeAddNodePopup");
    }

    inline void DrawGraphPopup(
        GraphPopupState& state,
        NodeGraph& graph)
    {
        ImGui::SetNextWindowPos(
            state.screenPosition,
            ImGuiCond_Appearing
        );

        if (!ImGui::BeginPopup(
            "CNodeAddNodePopup",
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings))
        {
            return;
        }

        if (ImGui::BeginMenu("3D Object"))
        {
            if (ImGui::MenuItem("Cube"))
            {
                AddCubeNode(
                    graph,
                    state.graphPosition.x,
                    state.graphPosition.y
                );
            }

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}
