
#pragma once

#include <algorithm>

#include "imgui.h"

namespace CNode
{
    struct GraphView
    {
        float zoom = 1.0f;
        ImVec2 offset = ImVec2(0.0f, 0.0f);
    };

    inline ImVec2 ScreenToGraph(
        const ImVec2& screen,
        const ImVec2& origin,
        const GraphView& view)
    {
        return ImVec2(
            (screen.x - origin.x - view.offset.x) / view.zoom,
            (screen.y - origin.y - view.offset.y) / view.zoom
        );
    }

    inline ImVec2 GraphToScreen(
        const ImVec2& position,
        const ImVec2& origin,
        const GraphView& view)
    {
        return ImVec2(
            origin.x + position.x * view.zoom + view.offset.x,
            origin.y + position.y * view.zoom + view.offset.y
        );
    }

    inline void UpdateGraphZoom(
        GraphView& view,
        const ImVec2& mouse,
        const ImVec2& origin,
        float wheel)
    {
        if (wheel == 0.0f)
            return;

        const ImVec2 graphPosition =
            ScreenToGraph(mouse, origin, view);

        const float zoomFactor =
            wheel > 0.0f ? 1.15f : 1.0f / 1.15f;

        view.zoom = std::clamp(
            view.zoom * zoomFactor,
            0.20f,
            2.50f
        );

        view.offset.x =
            mouse.x - origin.x -
            graphPosition.x * view.zoom;

        view.offset.y =
            mouse.y - origin.y -
            graphPosition.y * view.zoom;
    }
}
