
#pragma once

#include <algorithm>
#include "imgui.h"

#include "tab/windows/node_graph/node_graph_data.h"
#include "tab/windows/node_graph/graph_zoom.h"

namespace CNode
{
    struct GraphSelectionState
    {
        bool boxSelecting = false;
        bool additive = false;

        bool draggingNode = false;
        NodeID draggedNodeID = 0;

        ImVec2 start{ 0.0f, 0.0f };
        ImVec2 current{ 0.0f, 0.0f };

        ImVec2 dragStartMouse{ 0.0f, 0.0f };
        ImVec2 dragStartNode{ 0.0f, 0.0f };
    };

    inline bool IsPointInsideRect(
        const ImVec2& point,
        const ImVec2& rectMin,
        const ImVec2& rectMax)
    {
        return point.x >= rectMin.x &&
            point.x <= rectMax.x &&
            point.y >= rectMin.y &&
            point.y <= rectMax.y;
    }

    inline bool IsNodeInsideSelection(
        const GraphNode& node,
        const GraphView& view,
        const ImVec2& canvasOrigin,
        const ImVec2& selectionMin,
        const ImVec2& selectionMax)
    {
        const ImVec2 nodeMin = GraphToScreen(
            ImVec2(node.x, node.y),
            canvasOrigin,
            view
        );

        const ImVec2 nodeMax(
            nodeMin.x + node.width * view.zoom,
            nodeMin.y + node.height * view.zoom
        );

        // Node harus sepenuhnya berada di dalam kotak.
        return nodeMin.x >= selectionMin.x &&
            nodeMin.y >= selectionMin.y &&
            nodeMax.x <= selectionMax.x &&
            nodeMax.y <= selectionMax.y;
    }

    inline GraphNode* FindNodeAtScreenPosition(
        NodeGraph& graph,
        const GraphView& view,
        const ImVec2& canvasOrigin,
        const ImVec2& mouse)
    {
        // Node paling akhir digambar di atas node sebelumnya.
        for (auto it = graph.nodes.rbegin();
            it != graph.nodes.rend(); ++it)
        {
            GraphNode& node = *it;

            const ImVec2 nodeMin = GraphToScreen(
                ImVec2(node.x, node.y),
                canvasOrigin,
                view
            );

            const ImVec2 nodeMax(
                nodeMin.x + node.width * view.zoom,
                nodeMin.y + node.height * view.zoom
            );

            if (IsPointInsideRect(mouse, nodeMin, nodeMax))
                return &node;
        }

        return nullptr;
    }

    inline void UpdateGraphSelection(
        NodeGraph& graph,
        GraphSelectionState& state,
        const GraphView& view,
        const ImVec2& canvasOrigin,
        const ImVec2& mouse,
        bool canvasHovered)
    {
        const bool ctrl =
            ImGui::GetIO().KeyCtrl;

        // -------------------------------------------------
        // DRAG NODE
        // -------------------------------------------------

        if (state.draggingNode)
        {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                const ImVec2 mouseGraph = ScreenToGraph(
                    mouse,
                    canvasOrigin,
                    view
                );

                const ImVec2 startGraph = ScreenToGraph(
                    state.dragStartMouse,
                    canvasOrigin,
                    view
                );

                const float dx =
                    mouseGraph.x - startGraph.x;

                const float dy =
                    mouseGraph.y - startGraph.y;

                GraphNode* draggedNode = nullptr;

                for (auto& node : graph.nodes)
                {
                    if (node.id == state.draggedNodeID)
                    {
                        draggedNode = &node;
                        break;
                    }
                }

                if (draggedNode)
                {
                    const float newX =
                        state.dragStartNode.x + dx;

                    const float newY =
                        state.dragStartNode.y + dy;

                    const float moveX =
                        newX - draggedNode->x;

                    const float moveY =
                        newY - draggedNode->y;

                    // Pindahkan semua node yang dipilih
                    // bersama node yang sedang diseret.
                    for (auto& node : graph.nodes)
                    {
                        if (node.selected)
                        {
                            node.x += moveX;
                            node.y += moveY;
                        }
                    }
                }
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                state.draggingNode = false;
                state.draggedNodeID = 0;
            }

            return;
        }

        // -------------------------------------------------
        // BOX SELECTION YANG SEDANG BERJALAN
        // -------------------------------------------------

        if (state.boxSelecting)
        {
            state.current = mouse;

            const ImVec2 selectionMin(
                std::min(state.start.x, state.current.x),
                std::min(state.start.y, state.current.y)
            );

            const ImVec2 selectionMax(
                std::max(state.start.x, state.current.x),
                std::max(state.start.y, state.current.y)
            );

            for (auto& node : graph.nodes)
            {
                const bool inside = IsNodeInsideSelection(
                    node,
                    view,
                    canvasOrigin,
                    selectionMin,
                    selectionMax
                );

                if (state.additive)
                {
                    // Node yang sudah dipilih tetap dipilih.
                    if (inside)
                        node.selected = true;
                }
                else
                {
                    node.selected = inside;
                }
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                state.boxSelecting = false;

            return;
        }

        // Jangan mulai seleksi baru jika mouse tidak
        // berada di canvas atau bukan klik kiri baru.
        if (!canvasHovered ||
            !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            return;
        }

        GraphNode* hitNode = FindNodeAtScreenPosition(
            graph,
            view,
            canvasOrigin,
            mouse
        );

        // -------------------------------------------------
        // KLIK NODE
        // -------------------------------------------------

        if (hitNode)
        {
            if (ctrl)
            {
                hitNode->selected = !hitNode->selected;

                // Ctrl + klik untuk menghapus pilihan:
                // jangan mulai drag jika node tidak dipilih.
                if (!hitNode->selected)
                    return;
            }
            else if (!hitNode->selected)
            {
                for (auto& node : graph.nodes)
                    node.selected = false;

                hitNode->selected = true;
            }

            // Klik node yang sudah dipilih mempertahankan
            // pilihan grup dan memulai drag.
            if (hitNode->selected)
            {
                state.draggingNode = true;
                state.draggedNodeID = hitNode->id;
                state.dragStartMouse = mouse;

                state.dragStartNode = ImVec2(
                    hitNode->x,
                    hitNode->y
                );
            }

            return;
        }

        // -------------------------------------------------
        // KLIK AREA KOSONG: MULAI BOX SELECTION
        // -------------------------------------------------

        state.boxSelecting = true;
        state.additive = ctrl;
        state.start = mouse;
        state.current = mouse;

        if (!ctrl)
        {
            for (auto& node : graph.nodes)
                node.selected = false;
        }
    }

    inline void DrawGraphSelectionBox(
        const GraphSelectionState& state,
        ImDrawList* drawList)
    {
        if (!state.boxSelecting)
            return;

        const ImVec2 rectMin(
            std::min(state.start.x, state.current.x),
            std::min(state.start.y, state.current.y)
        );

        const ImVec2 rectMax(
            std::max(state.start.x, state.current.x),
            std::max(state.start.y, state.current.y)
        );

        drawList->AddRectFilled(
            rectMin,
            rectMax,
            IM_COL32(80, 145, 230, 45)
        );

        drawList->AddRect(
            rectMin,
            rectMax,
            IM_COL32(100, 165, 255, 230),
            0.0f,
            0,
            1.5f
        );
    }
}
