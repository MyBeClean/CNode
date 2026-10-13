#pragma once

#include "scene_projection.h"

namespace CNode::SceneEditor
{
    struct GizmoAxis
    {
        const char* label;
        Vec3 direction;
        ImU32 color;
        bool showLabel;
    };

    inline bool DrawViewGizmo(ImDrawList* drawList, ImVec2 center, Camera& camera, bool hovered)
    {
        constexpr float radius = 48.0f;
        drawList->AddCircleFilled(center, radius+8.0f, IM_COL32(25,28,35,220));
        drawList->AddCircle(center, radius+8.0f, IM_COL32(100,108,125,180));

        const GizmoAxis axes[] = {
            {"X",{ 1,0,0},IM_COL32(245,75,75,255),true},
            {"", {-1,0,0},IM_COL32(190,55,55,255),false},
            {"Y",{0, 1,0},IM_COL32(80,225,120,255),true},
            {"",{0,-1,0},IM_COL32(55,175,90,255),false},
            {"Z",{0,0, 1},IM_COL32(80,165,255,255),true},
            {"",{0,0,-1},IM_COL32(55,120,210,255),false}
        };

        const CameraBasis basis = camera.Basis();
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        static bool pressedOnGizmo = false;
        static bool draggedGizmo = false;
        static ImVec2 lastMouse(0.0f, 0.0f);
        static ImVec2 pressMouse(0.0f, 0.0f);
        static Vec3 clickedDirection{0,0,1};
        static bool pressOnAxis = false;

        // A click on an axis snaps the view. Dragging anywhere on the gizmo orbits it.
        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            pressedOnGizmo = true;
            draggedGizmo = false;
            lastMouse = mouse;
            pressMouse = mouse;
            pressOnAxis = false;

            for (const GizmoAxis& axis : axes)
            {
                const float x = Dot(axis.direction, basis.right);
                const float y = Dot(axis.direction, basis.up);
                const ImVec2 point(center.x+x*radius, center.y-y*radius);
                const float dx = mouse.x-point.x;
                const float dy = mouse.y-point.y;
                if (dx*dx+dy*dy <= 14.0f*14.0f)
                {
                    clickedDirection = axis.direction;
                    pressOnAxis = true;
                    break;
                }
            }
        }

        if (pressedOnGizmo && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            const float totalX = mouse.x - pressMouse.x;
            const float totalY = mouse.y - pressMouse.y;
            if (totalX*totalX + totalY*totalY > 9.0f)
                draggedGizmo = true;

            if (draggedGizmo)
            {
                camera.Orbit(mouse.x-lastMouse.x, mouse.y-lastMouse.y);
                lastMouse = mouse;
            }
        }

        if (pressedOnGizmo && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            if (!draggedGizmo && pressOnAxis)
                camera.LookFromDirection(clickedDirection);
            pressedOnGizmo = false;
            pressOnAxis = false;
        }

        // Draw after input so the gizmo immediately reflects the new camera orientation.
        const CameraBasis drawBasis = camera.Basis();
        for (const GizmoAxis& axis : axes)
        {
            const float x = Dot(axis.direction, drawBasis.right);
            const float y = Dot(axis.direction, drawBasis.up);
            const ImVec2 point(center.x+x*radius, center.y-y*radius);
            const float depth = Dot(axis.direction, drawBasis.forward);
            const float dotRadius = depth > 0.0f ? 9.0f : 7.0f;

            drawList->AddLine(center, point, axis.color, depth > 0.0f ? 3.0f : 2.0f);
            drawList->AddCircleFilled(point, dotRadius, axis.color);
            drawList->AddCircle(point, dotRadius, IM_COL32(20,22,28,220), 0, 1.0f);

            if (axis.showLabel)
            {
                const ImVec2 textSize = ImGui::CalcTextSize(axis.label);
                drawList->AddText(ImVec2(point.x-textSize.x*0.5f, point.y-textSize.y*0.5f),
                                  IM_COL32(255,255,255,255), axis.label);
            }
        }

        drawList->AddCircleFilled(center, 4.0f, IM_COL32(235,235,245,255));
        return draggedGizmo;
    }
}
