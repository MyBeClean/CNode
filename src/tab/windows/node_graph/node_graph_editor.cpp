
#include "node_graph_editor.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

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

        bool connectingSocket = false;
        SocketRef pendingSocket{};

        struct SocketVisual
        {
            SocketRef ref;
            ImVec2 position{ 0.0f, 0.0f };
            std::string label;
            bool output = false;
        };

        bool IsOutput(SocketKind kind)
        {
            return kind == SocketKind::ClassOutput ||
                kind == SocketKind::VariableOutput;
        }

        bool IsInput(SocketKind kind)
        {
            return kind == SocketKind::ClassInput ||
                kind == SocketKind::VariableInput;
        }

        SocketType GetClassSocketType(const ClassInstance& cls)
        {
            if (cls.className == "Transform")
                return SocketType::TransformClass;

            if (cls.className == "Visual")
                return SocketType::VisualClass;

            return SocketType::Any;
        }

        ImU32 GetSocketColor(SocketType type)
        {
            switch (type)
            {
            case SocketType::Float:
                return IM_COL32(230, 190, 90, 255);

            case SocketType::Integer:
                return IM_COL32(100, 180, 245, 255);

            case SocketType::Boolean:
                return IM_COL32(220, 100, 120, 255);

            case SocketType::String:
                return IM_COL32(100, 205, 150, 255);

            case SocketType::Vector3:
                return IM_COL32(110, 175, 245, 255);

            case SocketType::Material:
                return IM_COL32(195, 120, 235, 255);

            case SocketType::TransformClass:
                return IM_COL32(90, 165, 255, 255);

            case SocketType::VisualClass:
                return IM_COL32(235, 160, 85, 255);

            default:
                return IM_COL32(190, 190, 200, 255);
            }
        }

        bool SameSocket(const SocketRef& a, const SocketRef& b)
        {
            return a.nodeID == b.nodeID &&
                a.classID == b.classID &&
                a.variableID == b.variableID &&
                a.kind == b.kind;
        }

        bool CanConnect(const SocketRef& source, const SocketRef& target)
        {
            if (!IsOutput(source.kind) || !IsInput(target.kind))
                return false;

            if (source.nodeID == target.nodeID &&
                source.classID == target.classID &&
                source.variableID == target.variableID)
            {
                return false;
            }

            if (!AreSocketTypesCompatible(source.type, target.type))
                return false;

            const bool sourceIsClass =
                source.kind == SocketKind::ClassOutput;

            const bool targetIsClass =
                target.kind == SocketKind::ClassInput;

            // Class sockets hanya boleh dihubungkan ke class sockets.
            if (sourceIsClass != targetIsClass)
                return false;

            return true;
        }

        bool SocketExists(const NodeGraph& g, const SocketRef& socket)
        {
            const GraphNode* node = FindNode(g, socket.nodeID);

            if (!node)
                return false;

            for (const auto& cls : node->classes)
            {
                if (cls.id != socket.classID)
                    continue;

                if (socket.kind == SocketKind::ClassInput)
                    return !cls.classInputConnected;

                if (socket.kind == SocketKind::ClassOutput)
                    return IsClassOutputAvailable(cls);

                for (const auto& variable : cls.variables)
                {
                    if (variable.id != socket.variableID)
                        continue;

                    if (socket.kind == SocketKind::VariableInput)
                        return variable.hasInputSocket &&
                        !cls.classInputConnected;

                    if (socket.kind == SocketKind::VariableOutput)
                        return variable.hasOutputSocket;
                }
            }

            return false;
        }

        bool AddConnection(
            NodeGraph& g,
            const SocketRef& source,
            const SocketRef& target)
        {
            if (!SocketExists(g, source) ||
                !SocketExists(g, target) ||
                !CanConnect(source, target))
            {
                return false;
            }

            // Satu input hanya menerima satu koneksi.
            g.connections.erase(
                std::remove_if(
                    g.connections.begin(),
                    g.connections.end(),
                    [&](const Connection& connection)
                    {
                        return SameSocket(connection.target, target);
                    }),
                g.connections.end()
            );

            if (target.kind == SocketKind::ClassInput)
            {
                if (!ConnectClassInput(
                    g,
                    target.nodeID,
                    target.classID))
                {
                    return false;
                }
            }

            Connection connection;
            connection.source = source;
            connection.target = target;

            g.connections.push_back(connection);
            return true;
        }


        bool RemoveConnectionAtSocket(
            NodeGraph& g,
            const SocketRef& socket)
        {
            bool removed = false;

            // Jangan hapus koneksi sebelum memeriksa targetnya.
            // Kita perlu mengetahui class input mana yang harus
            // dikembalikan ke variable mode.
            for (auto it = g.connections.begin();
                it != g.connections.end();)
            {
                const bool matchesSource =
                    SameSocket(it->source, socket);

                const bool matchesTarget =
                    SameSocket(it->target, socket);

                if (!matchesSource && !matchesTarget)
                {
                    ++it;
                    continue;
                }

                // Kabel class yang dihapus dari sisi output tetap
                // harus memutus status input pada class target.
                if (it->target.kind == SocketKind::ClassInput)
                {
                    DisconnectClassInput(
                        g,
                        it->target.nodeID,
                        it->target.classID
                    );
                }

                it = g.connections.erase(it);
                removed = true;
            }

            return removed;
        }


        void BuildSocketVisuals(
            NodeGraph& g,
            const ImVec2& canvasOrigin,
            const GraphView& view,
            std::vector<SocketVisual>& sockets)
        {
            sockets.clear();

            const float zoom = view.zoom;

            for (auto& node : g.nodes)
            {
                const ImVec2 nodeScreen = GraphToScreen(
                    ImVec2(node.x, node.y),
                    canvasOrigin,
                    view
                );

                float y = 48.0f;

                for (auto& cls : node.classes)
                {
                    const float classCenterY = y + 15.0f;

                    const SocketType classType =
                        GetClassSocketType(cls);

                    if (!cls.classInputConnected)
                    {
                        SocketVisual socket;
                        socket.ref.nodeID = node.id;
                        socket.ref.classID = cls.id;
                        socket.ref.variableID = 0;
                        socket.ref.kind = SocketKind::ClassInput;
                        socket.ref.type = classType;
                        socket.position = ImVec2(
                            nodeScreen.x,
                            nodeScreen.y + classCenterY * zoom
                        );
                        socket.label = cls.className;
                        socket.output = false;

                        sockets.push_back(socket);
                    }

                    if (IsClassOutputAvailable(cls))
                    {
                        SocketVisual socket;
                        socket.ref.nodeID = node.id;
                        socket.ref.classID = cls.id;
                        socket.ref.variableID = 0;
                        socket.ref.kind = SocketKind::ClassOutput;
                        socket.ref.type = classType;
                        socket.position = ImVec2(
                            nodeScreen.x + node.width * zoom,
                            nodeScreen.y + classCenterY * zoom
                        );
                        socket.label = cls.className;
                        socket.output = true;

                        sockets.push_back(socket);
                    }

                    y += 36.0f;

                    if (cls.expanded &&
                        cls.hasVariables &&
                        !cls.classInputConnected)
                    {
                        for (const auto& variable : cls.variables)
                        {
                            const float variableCenterY = y + 10.0f;

                            if (variable.hasInputSocket)
                            {
                                SocketVisual socket;
                                socket.ref.nodeID = node.id;
                                socket.ref.classID = cls.id;
                                socket.ref.variableID = variable.id;
                                socket.ref.kind =
                                    SocketKind::VariableInput;
                                socket.ref.type = variable.type;
                                socket.position = ImVec2(
                                    nodeScreen.x,
                                    nodeScreen.y +
                                    variableCenterY * zoom
                                );
                                socket.label = variable.name;
                                socket.output = false;

                                sockets.push_back(socket);
                            }

                            if (variable.hasOutputSocket)
                            {
                                SocketVisual socket;
                                socket.ref.nodeID = node.id;
                                socket.ref.classID = cls.id;
                                socket.ref.variableID = variable.id;
                                socket.ref.kind =
                                    SocketKind::VariableOutput;
                                socket.ref.type = variable.type;
                                socket.position = ImVec2(
                                    nodeScreen.x + node.width * zoom,
                                    nodeScreen.y +
                                    variableCenterY * zoom
                                );
                                socket.label = variable.name;
                                socket.output = true;

                                sockets.push_back(socket);
                            }

                            y += 22.0f;
                        }
                    }

                    y += 2.0f;
                }

                // Node menyesuaikan tinggi isi agar socket tidak keluar.
                node.height = std::max(160.0f, y + 12.0f);
            }
        }

        SocketVisual* FindSocketAtMouse(
            std::vector<SocketVisual>& sockets,
            const ImVec2& mouse,
            float radius)
        {
            for (auto it = sockets.rbegin();
                it != sockets.rend(); ++it)
            {
                const float dx = mouse.x - it->position.x;
                const float dy = mouse.y - it->position.y;

                if (dx * dx + dy * dy <= radius * radius)
                    return &(*it);
            }

            return nullptr;
        }

        void DrawGraphGrid(
            ImDrawList* drawList,
            const ImVec2& canvasMin,
            const ImVec2& canvasMax,
            const GraphView& view)
        {
            const float spacing = 32.0f * view.zoom;

            if (spacing < 8.0f)
                return;

            float startX = std::fmod(view.offset.x, spacing);
            float startY = std::fmod(view.offset.y, spacing);

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

        void DrawConnections(
            ImDrawList* drawList,
            const NodeGraph& g,
            const std::vector<SocketVisual>& sockets)
        {
            auto findPosition = [&](const SocketRef& ref, ImVec2& position)
                {
                    for (const auto& socket : sockets)
                    {
                        if (SameSocket(socket.ref, ref))
                        {
                            position = socket.position;
                            return true;
                        }
                    }

                    return false;
                };

            for (const auto& connection : g.connections)
            {
                ImVec2 start;
                ImVec2 end;

                if (!findPosition(connection.source, start) ||
                    !findPosition(connection.target, end))
                {
                    continue;
                }

                const float curve = std::max(
                    40.0f,
                    std::abs(end.x - start.x) * 0.5f
                );

                const ImU32 color =
                    GetSocketColor(connection.source.type);

                drawList->AddBezierCubic(
                    start,
                    ImVec2(start.x + curve, start.y),
                    ImVec2(end.x - curve, end.y),
                    end,
                    color,
                    3.0f
                );
            }

            if (connectingSocket)
            {
                ImVec2 start;

                if (findPosition(pendingSocket, start))
                {
                    const ImVec2 mouse = ImGui::GetIO().MousePos;
                    const float curve = std::max(
                        40.0f,
                        std::abs(mouse.x - start.x) * 0.5f
                    );

                    drawList->AddBezierCubic(
                        start,
                        ImVec2(start.x + curve, start.y),
                        ImVec2(mouse.x - curve, mouse.y),
                        mouse,
                        GetSocketColor(pendingSocket.type),
                        2.0f
                    );
                }
            }
        }

        void DrawGraphNodes(
            ImDrawList* drawList,
            const ImVec2& canvasOrigin,
            const GraphView& view,
            const std::vector<SocketVisual>& sockets)
        {
            const float zoom = view.zoom;
            const float fontSize = ImGui::GetFontSize() * zoom;

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

                const ImU32 backgroundColor = node.selected
                    ? IM_COL32(63, 75, 100, 255)
                    : IM_COL32(53, 55, 62, 255);

                const ImU32 borderColor = node.selected
                    ? IM_COL32(100, 165, 255, 255)
                    : IM_COL32(100, 105, 120, 255);

                drawList->AddRectFilled(
                    nodeMin, nodeMax, backgroundColor, 7.0f * zoom
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

                const float headerY = nodeMin.y + 36.0f * zoom;

                drawList->AddLine(
                    ImVec2(nodeMin.x, headerY),
                    ImVec2(nodeMax.x, headerY),
                    IM_COL32(100, 105, 120, 255)
                );

                float classY = nodeMin.y + 48.0f * zoom;

                for (const auto& cls : node.classes)
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
                        cls.className.c_str()
                    );

                    classY += 36.0f * zoom;

                    if (cls.expanded &&
                        cls.hasVariables &&
                        !cls.classInputConnected)
                    {
                        for (const auto& variable : cls.variables)
                        {
                            drawList->AddText(
                                ImGui::GetFont(),
                                fontSize * 0.9f,
                                ImVec2(
                                    nodeMin.x + 12.0f * zoom,
                                    nodeMin.y + classY
                                ),
                                IM_COL32(185, 195, 210, 255),
                                variable.name.c_str()
                            );

                            classY += 22.0f * zoom;
                        }
                    }

                    classY += 2.0f * zoom;
                }
            }

            for (const auto& socket : sockets)
            {
                const float radius = 5.5f * zoom;
                const ImU32 color = GetSocketColor(socket.ref.type);

                drawList->AddCircleFilled(
                    socket.position,
                    radius + 2.0f * zoom,
                    IM_COL32(25, 26, 30, 255)
                );

                drawList->AddCircleFilled(
                    socket.position,
                    radius,
                    color
                );

                drawList->AddCircle(
                    socket.position,
                    radius,
                    IM_COL32(230, 235, 245, 230),
                    12,
                    1.0f
                );

                const ImVec2 labelPosition(
                    socket.position.x +
                    (socket.output ? -8.0f : 8.0f) * zoom,
                    socket.position.y - 7.0f * zoom
                );

                if (socket.output)
                {
                    const ImVec2 textSize =
                        ImGui::CalcTextSize(socket.label.c_str());

                    drawList->AddText(
                        ImGui::GetFont(),
                        fontSize * 0.8f,
                        ImVec2(
                            labelPosition.x - textSize.x * zoom,
                            labelPosition.y
                        ),
                        IM_COL32(190, 200, 215, 255),
                        socket.label.c_str()
                    );
                }
                else
                {
                    drawList->AddText(
                        ImGui::GetFont(),
                        fontSize * 0.8f,
                        labelPosition,
                        IM_COL32(190, 200, 215, 255),
                        socket.label.c_str()
                    );
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

        connectingSocket = false;
        pendingSocket = SocketRef{};
    }

    void DrawNodeGraphEditor(const char* title, bool* open)
    {
        if (!ImGui::Begin(title, open))
        {
            ImGui::End();
            return;
        }

        if (ImGui::Button("+"))
        {
            // Simpan posisi mouse untuk lokasi popup.
            const ImVec2 mouse = ImGui::GetIO().MousePos;

            // Tombol + berada di toolbar, jadi gunakan posisi
            // canvas yang sebenarnya untuk menghitung koordinat graph.
            const ImVec2 canvasOrigin =
                ImGui::GetCursorScreenPos();

            const ImVec2 graphPosition = ScreenToGraph(
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

        ImGui::SameLine();
        ImGui::TextUnformatted("Node Graph");
        ImGui::SameLine();

        ImGui::TextDisabled("%.0f%%", graphView.zoom * 100.0f);
        ImGui::SameLine();

        if (ImGui::SmallButton("Reset Zoom"))
        {
            graphView.zoom = 1.0f;
            graphView.offset = ImVec2(0.0f, 0.0f);
        }

        ImGui::SameLine();

        if (ImGui::SmallButton("Cancel Connection"))
        {
            connectingSocket = false;
        }

        ImGui::Separator();

        const ImVec2 canvasOrigin = ImGui::GetCursorScreenPos();

        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        canvasSize.x = std::max(1.0f, canvasSize.x);
        canvasSize.y = std::max(1.0f, canvasSize.y);

        const ImVec2 canvasMax(
            canvasOrigin.x + canvasSize.x,
            canvasOrigin.y + canvasSize.y
        );

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        drawList->AddRectFilled(
            canvasOrigin,
            canvasMax,
            IM_COL32(35, 35, 39, 255)
        );

        DrawGraphGrid(drawList, canvasOrigin, canvasMax, graphView);

        ImGui::InvisibleButton(
            "##NodeGraphCanvas",
            canvasSize,
            ImGuiButtonFlags_MouseButtonLeft |
            ImGuiButtonFlags_MouseButtonRight
        );

        const bool canvasHovered = ImGui::IsItemHovered();
        const ImVec2 mouse = ImGui::GetIO().MousePos;

        if (canvasHovered)
        {
            UpdateGraphZoom(
                graphView,
                mouse,
                canvasOrigin,
                ImGui::GetIO().MouseWheel
            );
        }

        std::vector<SocketVisual> sockets;
        BuildSocketVisuals(graph, canvasOrigin, graphView, sockets);

        bool socketConsumed = false;
        SocketVisual* hoveredSocket = nullptr;

        if (canvasHovered)
        {
            hoveredSocket = FindSocketAtMouse(
                sockets,
                mouse,
                std::max(8.0f, 10.0f * graphView.zoom)
            );
        }

        if (hoveredSocket &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            RemoveConnectionAtSocket(graph, hoveredSocket->ref);
            connectingSocket = false;
            socketConsumed = true;
        }

        if (hoveredSocket &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            socketConsumed = true;

            if (connectingSocket)
            {
                if (CanConnect(pendingSocket, hoveredSocket->ref))
                {
                    AddConnection(
                        graph,
                        pendingSocket,
                        hoveredSocket->ref
                    );

                    connectingSocket = false;
                }
                else if (hoveredSocket->output)
                {
                    pendingSocket = hoveredSocket->ref;
                }
                else
                {
                    connectingSocket = false;
                }
            }
            else if (hoveredSocket->output)
            {
                pendingSocket = hoveredSocket->ref;
                connectingSocket = true;
            }
        }

        if (connectingSocket &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
            !hoveredSocket)
        {
            connectingSocket = false;
        }

        if (!socketConsumed)
        {
            UpdateGraphSelection(
                graph,
                selectionState,
                graphView,
                canvasOrigin,
                mouse,
                canvasHovered
            );
        }

        if (canvasHovered &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
            !hoveredSocket &&
            !connectingSocket)
        {
            const ImVec2 graphPosition = ScreenToGraph(
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

        drawList->PushClipRect(canvasOrigin, canvasMax, true);

        DrawConnections(drawList, graph, sockets);
        DrawGraphNodes(drawList, canvasOrigin, graphView, sockets);
        DrawGraphSelectionBox(selectionState, drawList);

        drawList->PopClipRect();

        DrawGraphPopup(popupState, graph);

        ImGui::End();
    }
}
