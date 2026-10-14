#include "node_graph_editor.h"

#include "imgui.h"

#include "tab/windows/node_graph/node_graph_data.h"
#include "node/3d_object/cube/cube.h"

namespace CNode
{
    namespace
    {
        // State sementara editor.
        // Penyimpanan graph permanen akan dihubungkan ke
        // sistem scene/object milik CNode.
        NodeGraph graph;

        bool showAddNodePopup = false;

        ImVec2 popupPosition = ImVec2(0.0f, 0.0f);

        void AddCubeAtPosition(const ImVec2& position)
        {
            AddCubeNode(
                graph,
                position.x,
                position.y
            );
        }

        void DrawAddNodePopup()
        {
            if (!showAddNodePopup)
                return;

            ImGui::SetNextWindowPos(
                popupPosition,
                ImGuiCond_Appearing
            );

            ImGui::SetNextWindowSize(
                ImVec2(220.0f, 0.0f)
            );

            if (ImGui::Begin(
                "Add Node###CNodeAddNodePopup",
                &showAddNodePopup,
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_AlwaysAutoResize))
            {
                if (ImGui::BeginMenu("3D Object"))
                {
                    if (ImGui::MenuItem("Cube"))
                    {
                        AddCubeAtPosition(
                            ImVec2(100.0f, 100.0f)
                        );

                        showAddNodePopup = false;
                    }

                    ImGui::EndMenu();
                }
            }

            ImGui::End();
        }
    }

    void DrawNodeGraphEditor(
        const char* title,
        bool* open)
    {
        if (!ImGui::Begin(title, open))
        {
            ImGui::End();
            return;
        }

        // -----------------------------------------------------
        // ADD BUTTON
        // -----------------------------------------------------

        if (ImGui::Button("+"))
        {
            popupPosition = ImGui::GetMousePos();
            showAddNodePopup = true;
        }

        ImGui::SameLine();

        ImGui::TextUnformatted("Node Graph");

        ImGui::Separator();

        // -----------------------------------------------------
        // GRAPH AREA
        // -----------------------------------------------------

        const ImVec2 canvasPosition =
            ImGui::GetCursorScreenPos();

        const ImVec2 canvasSize =
            ImGui::GetContentRegionAvail();

        ImGui::InvisibleButton(
            "##NodeGraphCanvas",
            canvasSize,
            ImGuiButtonFlags_MouseButtonLeft |
            ImGuiButtonFlags_MouseButtonRight
        );

        const bool canvasHovered =
            ImGui::IsItemHovered();

        const ImVec2 mousePosition =
            ImGui::GetIO().MousePos;

        // Klik kanan di area kosong membuka menu.
        if (canvasHovered &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            popupPosition = mousePosition;
            showAddNodePopup = true;
        }

        // -----------------------------------------------------
        // DRAW CANVAS
        // -----------------------------------------------------

        ImDrawList* drawList =
            ImGui::GetWindowDrawList();

        drawList->AddRectFilled(
            canvasPosition,
            ImVec2(
                canvasPosition.x + canvasSize.x,
                canvasPosition.y + canvasSize.y
            ),
            IM_COL32(35, 35, 39, 255)
        );

        // -----------------------------------------------------
        // DRAW NODES
        // -----------------------------------------------------

        for (const auto& node : graph.nodes)
        {
            const ImVec2 nodePosition(
                canvasPosition.x + node.x,
                canvasPosition.y + node.y
            );

            const ImVec2 nodeSize(
                node.width,
                node.height
            );

            const ImVec2 nodeEnd(
                nodePosition.x + nodeSize.x,
                nodePosition.y + nodeSize.y
            );

            drawList->AddRectFilled(
                nodePosition,
                nodeEnd,
                IM_COL32(53, 55, 62, 255),
                7.0f
            );

            drawList->AddRect(
                nodePosition,
                nodeEnd,
                IM_COL32(100, 105, 120, 255),
                7.0f
            );

            drawList->AddText(
                ImVec2(
                    nodePosition.x + 12.0f,
                    nodePosition.y + 10.0f
                ),
                IM_COL32(240, 240, 245, 255),
                node.name.c_str()
            );

            drawList->AddLine(
                ImVec2(
                    nodePosition.x,
                    nodePosition.y + 36.0f
                ),
                ImVec2(
                    nodeEnd.x,
                    nodePosition.y + 36.0f
                ),
                IM_COL32(100, 105, 120, 255)
            );

            float classY = nodePosition.y + 48.0f;

            for (const auto& classInstance : node.classes)
            {
                const ImVec2 classMin(
                    nodePosition.x + 8.0f,
                    classY
                );

                const ImVec2 classMax(
                    nodeEnd.x - 8.0f,
                    classY + 30.0f
                );

                drawList->AddRectFilled(
                    classMin,
                    classMax,
                    IM_COL32(42, 44, 50, 255),
                    4.0f
                );

                drawList->AddText(
                    ImVec2(
                        classMin.x + 8.0f,
                        classMin.y + 7.0f
                    ),
                    IM_COL32(220, 225, 235, 255),
                    classInstance.className.c_str()
                );

                classY += 36.0f;
            }
        }

        // -----------------------------------------------------
        // ADD NODE POPUP
        // -----------------------------------------------------

        DrawAddNodePopup();

        ImGui::End();
    }
}