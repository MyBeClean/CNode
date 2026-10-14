#include "node_graph_editor.h"

#include <algorithm>
#include <cmath>

#include "imgui.h"

#include "tab/windows/node_graph/node_graph_data.h"
#include "tab/windows/node_graph/graph_zoom.h"
#include "tab/windows/node_graph/graph_popup.h"
#include "tab/windows/node_graph/graph_selection.h"

namespace CNode
{
    namespace
    {
        NodeGraph graph;

        GraphView graphView;
        GraphPopupState popupState;
        GraphSelectionState selectionState;

        void DrawGraphGrid(
            ImDrawList* drawList,
            const ImVec2& canvasMin,
            const ImVec2& canvasMax,
            const GraphView& view)
        {
            const float spacing = 32.0f * view.zoom;

            if (spacing < 8.0f)
                return;

            float startX = std::fmod(
                view.offset.x,
                spacing
            );

            float startY = std::fmod(
                view.offset.y,
                spacing
            );

            if (startX < 0.0f)
                startX += spacing;

            if (startY < 0.0f)
                startY += spacing;

            startX += canvasMin.x;
            startY += canvasMin.y;

            for (float x = startX; x < canvasMax.x; x += spacing)
            {
                drawList->AddLine(
                    ImVec2(x, canvasMin.y),
                    ImVec2(x, canvasMax.y),
                    IM_COL32(48, 48, 53, 255)
                );
            }

            for (float y = startY; y < canvasMax.y; y += spacing)
            {
                drawList->AddLine(
                    ImVec2(canvasMin.x, y),
                    ImVec2(canvasMax.x, y),
                    IM_COL32(48, 48, 53, 255)
                );
            }
        }

        void DrawGraphNodes(
            ImDrawList* drawList,
            const ImVec2& canvasOrigin,
            const GraphView& view)
        {
            const float zoom = view.zoom;
            const float fontSize =
                ImGui::GetFontSize() * zoom;

            for (const auto& node : graph.nodes)
            {
                const ImVec2 nodeMin = GraphToScreen(
                    ImVec2(node.x, node.y),
                    canvasOrigin,
                    view
                );

                const ImVec2 nodeMax(
                    nodeMin.x + node.width * zoom,
                    nodeMin.y + node.height * zoom
                );

                const ImU32 backgroundColor =
                    node.selected
                    ? IM_COL32(63, 75, 100, 255)
                    : IM_COL32(53, 55, 62, 255);

                const ImU32 borderColor =
                    node.selected
                    ? IM_COL32(100, 165, 255, 255)
                    : IM_COL32(100, 105, 120, 255);

                drawList->AddRectFilled(
                    nodeMin,
                    nodeMax,
                    backgroundColor,
                    7.0f * zoom
                );

                drawList->AddRect(
                    nodeMin,
                    nodeMax,
                    borderColor,
                    7.0f * zoom,
                    0,
                    node.selected ? 2.0f : 1.0f
                );

                drawList->AddText(
                    ImGui::GetFont(),
                    fontSize,
                    ImVec2(
                        nodeMin.x + 12.0f * zoom,
                        nodeMin.y + 10.0f * zoom
                    ),
                    IM_COL32(240, 240, 245, 255),
                    node.name.c_str()
                );

                const float headerY =
                    nodeMin.y + 36.0f * zoom;

                drawList->AddLine(
                    ImVec2(nodeMin.x, headerY),
                    ImVec2(nodeMax.x, headerY),
                    IM_COL32(100, 105, 120, 255)
                );

                float classY =
                    nodeMin.y + 48.0f * zoom;

                for (const auto& classInstance : node.classes)
                {
                    const ImVec2 classMin(
                        nodeMin.x + 8.0f * zoom,
                        classY
                    );

                    const ImVec2 classMax(
                        nodeMax.x - 8.0f * zoom,
                        classY + 30.0f * zoom
                    );

                    drawList->AddRectFilled(
                        classMin,
                        classMax,
                        IM_COL32(42, 44, 50, 255),
                        4.0f * zoom
                    );

                    drawList->AddText(
                        ImGui::GetFont(),
                        fontSize,
                        ImVec2(
                            classMin.x + 8.0f * zoom,
                            classMin.y + 7.0f * zoom
                        ),
                        IM_COL32(220, 225, 235, 255),
                        classInstance.className.c_str()
                    );

                    classY += 36.0f * zoom;
                }
            }
        }
    }

    NodeGraph& GetActiveNodeGraph()
    {
        return graph;
    }

    void ResetNodeGraph()
    {
        graph = NodeGraph{};

        graphView.zoom = 1.0f;
        graphView.offset = ImVec2(0.0f, 0.0f);

        popupState = GraphPopupState{};
        selectionState = GraphSelectionState{};
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

        if (ImGui::Button("+"))
        {
            const ImVec2 mouse =
                ImGui::GetIO().MousePos;

            OpenGraphPopup(
                popupState,
                mouse,
                ImVec2(100.0f, 100.0f)
            );
        }

        ImGui::SameLine();

        ImGui::TextUnformatted("Node Graph");

        ImGui::SameLine();

        ImGui::TextDisabled(
            "%.0f%%",
            graphView.zoom * 100.0f
        );

        ImGui::SameLine();

        if (ImGui::SmallButton("Reset Zoom"))
        {
            graphView.zoom = 1.0f;
            graphView.offset = ImVec2(0.0f, 0.0f);
        }

        ImGui::Separator();

        const ImVec2 canvasOrigin =
            ImGui::GetCursorScreenPos();

        ImVec2 canvasSize =
            ImGui::GetContentRegionAvail();

        canvasSize.x = std::max(1.0f, canvasSize.x);
        canvasSize.y = std::max(1.0f, canvasSize.y);

        const ImVec2 canvasMax(
            canvasOrigin.x + canvasSize.x,
            canvasOrigin.y + canvasSize.y
        );

        ImDrawList* drawList =
            ImGui::GetWindowDrawList();

        drawList->AddRectFilled(
            canvasOrigin,
            canvasMax,
            IM_COL32(35, 35, 39, 255)
        );

        DrawGraphGrid(
            drawList,
            canvasOrigin,
            canvasMax,
            graphView
        );

        ImGui::InvisibleButton(
            "##NodeGraphCanvas",
            canvasSize,
            ImGuiButtonFlags_MouseButtonLeft |
            ImGuiButtonFlags_MouseButtonRight
        );

        const bool canvasHovered =
            ImGui::IsItemHovered();

        const ImVec2 mouse =
            ImGui::GetIO().MousePos;

        if (canvasHovered)
        {
            UpdateGraphZoom(
                graphView,
                mouse,
                canvasOrigin,
                ImGui::GetIO().MouseWheel
            );
        }

        UpdateGraphSelection(
            graph,
            selectionState,
            graphView,
            canvasOrigin,
            mouse,
            canvasHovered
        );

        if (canvasHovered &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            const ImVec2 graphPosition =
                ScreenToGraph(
                    mouse,
                    canvasOrigin,
                    graphView
                );

            OpenGraphPopup(
                popupState,
                mouse,
                graphPosition
            );
        }

        drawList->PushClipRect(
            canvasOrigin,
            canvasMax,
            true
        );

        DrawGraphNodes(
            drawList,
            canvasOrigin,
            graphView
        );

        DrawGraphSelectionBox(
            selectionState,
            drawList
        );

        drawList->PopClipRect();

        DrawGraphPopup(
            popupState,
            graph
        );

        ImGui::End();
    }
}