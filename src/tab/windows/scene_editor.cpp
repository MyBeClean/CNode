
#include "scene_editor.h"
#include "environment/environment_settings.h"

#include "imgui.h"

#include <algorithm>
#include <cmath>

namespace CNode
{
    namespace
    {
        struct Vec3
        {
            float x, y, z;
        };

        static constexpr float PI = 3.14159265358979323846f;

        static float cameraYaw = 0.75f;
        static float cameraPitch = 0.45f;
        static float cameraDistance = 12.0f;

        // Titik yang dilihat kamera.
        static Vec3 cameraPivot = { 0.0f, 0.0f, 0.0f };

        static bool selecting = false;
        static ImVec2 selectionStart(0, 0);
        static ImVec2 selectionEnd(0, 0);

        static bool orbitingGizmo = false;
        static ImVec2 gizmoDragStart(0, 0);
        static float gizmoStartYaw = 0.0f;
        static float gizmoStartPitch = 0.0f;

        static Vec3 Add(Vec3 a, Vec3 b)
        {
            return { a.x + b.x, a.y + b.y, a.z + b.z };
        }

        static Vec3 Subtract(Vec3 a, Vec3 b)
        {
            return { a.x - b.x, a.y - b.y, a.z - b.z };
        }

        static Vec3 Multiply(Vec3 v, float s)
        {
            return { v.x * s, v.y * s, v.z * s };
        }

        static float Dot(Vec3 a, Vec3 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        static Vec3 Cross(Vec3 a, Vec3 b)
        {
            return {
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            };
        }

        static Vec3 Normalize(Vec3 v)
        {
            const float length = std::sqrt(Dot(v, v));

            if (length < 0.0001f)
                return { 0.0f, 0.0f, 0.0f };

            return Multiply(v, 1.0f / length);
        }

        static Vec3 GetCameraPosition()
        {
            const float cp = std::cos(cameraPitch);

            return Add(cameraPivot, {
                cameraDistance * cp * std::sin(cameraYaw),
                cameraDistance * std::sin(cameraPitch),
                cameraDistance * cp * std::cos(cameraYaw)
                });
        }

        struct CameraBasis
        {
            Vec3 position;
            Vec3 forward;
            Vec3 right;
            Vec3 up;
        };

        static CameraBasis GetCameraBasis()
        {
            CameraBasis camera;

            camera.position = GetCameraPosition();

            camera.forward = Normalize(
                Subtract(cameraPivot, camera.position)
            );

            const Vec3 worldUp = { 0.0f, 1.0f, 0.0f };

            camera.right = Normalize(
                Cross(camera.forward, worldUp)
            );

            camera.up = Normalize(
                Cross(camera.right, camera.forward)
            );

            return camera;
        }

        static ImVec2 ProjectPoint(
            Vec3 point,
            ImVec2 center,
            float width,
            float height)
        {
            const CameraBasis camera = GetCameraBasis();

            const Vec3 relative =
                Subtract(point, camera.position);

            const float depth = Dot(relative, camera.forward);
            const float cameraX = Dot(relative, camera.right);
            const float cameraY = Dot(relative, camera.up);

            const float focal = std::min(width, height) * 0.9f;
            const float safeDepth = std::max(depth, 0.1f);

            return {
                center.x + cameraX * focal / safeDepth,
                center.y - cameraY * focal / safeDepth
            };
        }

        static void DrawLine3D(
            ImDrawList* drawList,
            Vec3 a,
            Vec3 b,
            ImVec2 center,
            float width,
            float height,
            ImU32 color,
            float thickness = 1.0f)
        {
            drawList->AddLine(
                ProjectPoint(a, center, width, height),
                ProjectPoint(b, center, width, height),
                color,
                thickness
            );
        }

        static void DrawGrid(
            ImDrawList* drawList,
            ImVec2 center,
            float width,
            float height,
            bool showAxes)
        {
            const ImU32 gridColor =
                IM_COL32(90, 98, 112, 125);

            constexpr int gridSize = 20;

            for (int i = -gridSize; i <= gridSize; ++i)
            {
                const float p = static_cast<float>(i);

                DrawLine3D(
                    drawList,
                    { p, 0.0f, -20.0f },
                    { p, 0.0f, 20.0f },
                    center, width, height, gridColor
                );

                DrawLine3D(
                    drawList,
                    { -20.0f, 0.0f, p },
                    { 20.0f, 0.0f, p },
                    center, width, height, gridColor
                );
            }

            if (showAxes)
            {
                DrawLine3D(
                    drawList,
                    { -100.0f, 0.0f, 0.0f },
                    { 100.0f, 0.0f, 0.0f },
                    center, width, height,
                    IM_COL32(245, 75, 75, 255), 2.0f
                );

                DrawLine3D(
                    drawList,
                    { 0.0f, -100.0f, 0.0f },
                    { 0.0f, 100.0f, 0.0f },
                    center, width, height,
                    IM_COL32(80, 225, 120, 255), 2.0f
                );

                DrawLine3D(
                    drawList,
                    { 0.0f, 0.0f, -100.0f },
                    { 0.0f, 0.0f, 100.0f },
                    center, width, height,
                    IM_COL32(80, 165, 255, 255), 2.0f
                );
            }
        }

        static void DrawViewGizmo(
            ImDrawList* drawList,
            ImVec2 viewportMin,
            float viewportWidth)
        {
            const float radius = 48.0f;

            const ImVec2 center(
                viewportMin.x + viewportWidth - 65.0f,
                viewportMin.y + 65.0f
            );

            drawList->AddCircleFilled(
                center, radius + 8.0f,
                IM_COL32(25, 28, 35, 220)
            );

            drawList->AddCircle(
                center, radius + 8.0f,
                IM_COL32(100, 108, 125, 180)
            );

            const CameraBasis camera = GetCameraBasis();

            struct Axis
            {
                const char* label;
                Vec3 direction;
                ImU32 color;
            };

            const Axis axes[] = {
                {"X", {1, 0, 0}, IM_COL32(245, 75, 75, 255)},
                {"Y", {0, 1, 0}, IM_COL32(80, 225, 120, 255)},
                {"Z", {0, 0, 1}, IM_COL32(80, 165, 255, 255)}
            };

            for (const Axis& axis : axes)
            {
                const float x = Dot(axis.direction, camera.right);
                const float y = Dot(axis.direction, camera.up);

                const ImVec2 end(
                    center.x + x * radius,
                    center.y - y * radius
                );

                drawList->AddLine(
                    center, end, axis.color, 3.0f
                );

                drawList->AddCircleFilled(
                    end, 10.0f, axis.color
                );

                const ImVec2 textSize =
                    ImGui::CalcTextSize(axis.label);

                drawList->AddText(
                    ImVec2(
                        end.x - textSize.x * 0.5f,
                        end.y - textSize.y * 0.5f
                    ),
                    IM_COL32(255, 255, 255, 255),
                    axis.label
                );
            }

            drawList->AddCircleFilled(
                center, 4.0f,
                IM_COL32(235, 235, 245, 255)
            );
        }

        static void OrbitCamera(float dx, float dy)
        {
            cameraYaw -= dx * 0.01f;
            cameraPitch += dy * 0.01f;

            cameraPitch = std::clamp(
                cameraPitch, -1.55f, 1.55f
            );
        }

        static void PanCamera(float dx, float dy)
        {
            const CameraBasis camera = GetCameraBasis();

            const float scale = cameraDistance * 0.002f;

            // Gerak kamera dan pivot bersama-sama.
            // Arah pandang dan jarak orbit tidak berubah.
            const Vec3 movement = Multiply(
                Add(
                    Multiply(camera.right, -dx),
                    Multiply(camera.up, dy)
                ),
                scale
            );

            cameraPivot = Add(cameraPivot, movement);
        }
    }

    void DrawSceneEditor(const char* title, bool* open)
    {
        ImGui::Begin(title, open);

        const ImVec2 available = ImGui::GetContentRegionAvail();

        if (available.x <= 1.0f || available.y <= 1.0f)
        {
            ImGui::End();
            return;
        }

        const ImVec2 start = ImGui::GetCursorScreenPos();

        const ImVec2 end(
            start.x + available.x,
            start.y + available.y
        );

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        const EnvironmentSettings& env =
            GetEnvironmentSettings();

        drawList->PushClipRect(start, end, true);

        drawList->AddRectFilled(
            start, end,
            ImGui::ColorConvertFloat4ToU32(env.backgroundColor)
        );

        ImGui::InvisibleButton(
            "SceneViewport",
            available,
            ImGuiButtonFlags_MouseButtonLeft |
            ImGuiButtonFlags_MouseButtonMiddle
        );

        const bool hovered = ImGui::IsItemHovered();
        const ImVec2 mouse = ImGui::GetIO().MousePos;

        const ImVec2 gizmoCenter(
            end.x - 65.0f,
            start.y + 65.0f
        );

        const float gizmoRadius = 56.0f;

        const bool overGizmo =
            mouse.x >= gizmoCenter.x - gizmoRadius &&
            mouse.x <= gizmoCenter.x + gizmoRadius &&
            mouse.y >= gizmoCenter.y - gizmoRadius &&
            mouse.y <= gizmoCenter.y + gizmoRadius;

        if (hovered)
        {
            // Klik kiri di gizmo untuk memulai orbit.
            if (overGizmo &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                orbitingGizmo = true;
                gizmoDragStart = mouse;
                gizmoStartYaw = cameraYaw;
                gizmoStartPitch = cameraPitch;
                selecting = false;
            }

            // Box selection dimulai dari klik kiri di area kosong.
            if (!overGizmo &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                selecting = true;
                selectionStart = mouse;
                selectionEnd = mouse;
                orbitingGizmo = false;
            }

            if (selecting &&
                ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                selectionEnd = mouse;
            }

            // Tombol tengah: orbit.
            // Alt + tombol tengah: pan.
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
            {
                const ImVec2 delta = ImGui::GetIO().MouseDelta;

                if (ImGui::GetIO().KeyAlt)
                    PanCamera(delta.x, delta.y);
                else
                    OrbitCamera(delta.x, delta.y);
            }

            // Scroll wheel tanpa ditahan: zoom.
            if (ImGui::GetIO().MouseWheel != 0.0f &&
                !overGizmo)
            {
                cameraDistance = std::clamp(
                    cameraDistance - ImGui::GetIO().MouseWheel,
                    1.0f, 100.0f
                );
            }
        }

        // Drag gizmo menggunakan kiri mouse.
        if (orbitingGizmo)
        {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                const ImVec2 delta(
                    mouse.x - gizmoDragStart.x,
                    mouse.y - gizmoDragStart.y
                );

                cameraYaw = gizmoStartYaw - delta.x * 0.01f;

                cameraPitch = std::clamp(
                    gizmoStartPitch + delta.y * 0.01f,
                    -1.55f, 1.55f
                );
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                orbitingGizmo = false;
        }

        // Origin diproyeksikan tepat di tengah viewport.
        const ImVec2 sceneCenter(
            start.x + available.x * 0.5f,
            start.y + available.y * 0.5f
        );

        if (env.showGrid)
        {
            DrawGrid(
                drawList,
                sceneCenter,
                available.x,
                available.y,
                env.showAxes
            );
        }
        else if (env.showAxes)
        {
            DrawLine3D(
                drawList,
                { -100, 0, 0 }, { 100, 0, 0 },
                sceneCenter, available.x, available.y,
                IM_COL32(245, 75, 75, 255), 2.0f
            );

            DrawLine3D(
                drawList,
                { 0, -100, 0 }, { 0, 100, 0 },
                sceneCenter, available.x, available.y,
                IM_COL32(80, 225, 120, 255), 2.0f
            );

            DrawLine3D(
                drawList,
                { 0, 0, -100 }, { 0, 0, 100 },
                sceneCenter, available.x, available.y,
                IM_COL32(80, 165, 255, 255), 2.0f
            );
        }

        DrawViewGizmo(drawList, start, available.x);

        drawList->AddText(
            ImVec2(start.x + 12.0f, start.y + 10.0f),
            IM_COL32(225, 230, 240, 230),
            "Scene View"
        );

        if (selecting)
        {
            const ImVec2 boxMin(
                std::min(selectionStart.x, selectionEnd.x),
                std::min(selectionStart.y, selectionEnd.y)
            );

            const ImVec2 boxMax(
                std::max(selectionStart.x, selectionEnd.x),
                std::max(selectionStart.y, selectionEnd.y)
            );

            drawList->AddRectFilled(
                boxMin, boxMax,
                IM_COL32(80, 150, 255, 45)
            );

            drawList->AddRect(
                boxMin, boxMax,
                IM_COL32(100, 175, 255, 240)
            );
        }

        if (selecting &&
            ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            selecting = false;

            // TODO: hubungkan ke sistem object picking
            // setelah scene memiliki objek 3D.
        }

        drawList->PopClipRect();

        ImGui::End();
    }
}
