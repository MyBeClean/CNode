#include "scene_editor.h"
#include "tab/windows/project_setting/environment/environment_settings.h"
#include "scene_camera.h"
#include "scene_grid.h"
#include "scene_gizmo.h"
#include "scene_viewport_renderer.h"
#include "tab/file/file_tab.h"

#include <cstdint>

#include "imgui.h"
#include "imgui_internal.h"

#include <GLFW/glfw3.h>
#include <algorithm>

namespace CNode
{
    namespace
    {
        using namespace SceneEditor;

        static Camera camera;

        static bool selecting = false;
        static ImVec2 selectionStart(0.0f, 0.0f);
        static ImVec2 selectionEnd(0.0f, 0.0f);

        static bool cursorLocked = false;
        static bool mouseCaptureActive = false;
        static bool gizmoDragging = false;

        static GLFWwindow* cursorWindow = nullptr;

        static bool IsAnyMouseButtonDown()
        {
            return
                ImGui::IsMouseDown(ImGuiMouseButton_Left) ||
                ImGui::IsMouseDown(ImGuiMouseButton_Right) ||
                ImGui::IsMouseDown(ImGuiMouseButton_Middle);
        }

        static bool IsAnyMouseButtonClicked()
        {
            return
                ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
                ImGui::IsMouseClicked(ImGuiMouseButton_Right) ||
                ImGui::IsMouseClicked(ImGuiMouseButton_Middle);
        }

        static GLFWwindow* GetSceneWindow()
        {
            ImGuiViewport* viewport = ImGui::GetWindowViewport();

            if (viewport == nullptr || viewport->PlatformHandle == nullptr)
                return nullptr;

            return static_cast<GLFWwindow*>(viewport->PlatformHandle);
        }

        static void SetCursorLocked(bool locked)
        {
            GLFWwindow* window = GetSceneWindow();

            if (window == nullptr)
                return;

            if (cursorLocked && cursorWindow != window)
            {
                if (cursorWindow != nullptr)
                {
                    glfwSetInputMode(
                        cursorWindow,
                        GLFW_RAW_MOUSE_MOTION,
                        GLFW_FALSE
                    );

                    glfwSetInputMode(
                        cursorWindow,
                        GLFW_CURSOR,
                        GLFW_CURSOR_NORMAL
                    );
                }

                cursorLocked = false;
                cursorWindow = nullptr;
            }

            if (cursorLocked == locked && cursorWindow == window)
                return;

            if (locked)
            {
                glfwSetInputMode(
                    window,
                    GLFW_CURSOR,
                    GLFW_CURSOR_DISABLED
                );

                if (glfwRawMouseMotionSupported())
                {
                    glfwSetInputMode(
                        window,
                        GLFW_RAW_MOUSE_MOTION,
                        GLFW_TRUE
                    );
                }

                cursorWindow = window;
                cursorLocked = true;
            }
            else
            {
                glfwSetInputMode(
                    window,
                    GLFW_RAW_MOUSE_MOTION,
                    GLFW_FALSE
                );

                glfwSetInputMode(
                    window,
                    GLFW_CURSOR,
                    GLFW_CURSOR_NORMAL
                );

                cursorWindow = nullptr;
                cursorLocked = false;
            }
        }

        static void ReleaseMouseCapture()
        {
            mouseCaptureActive = false;
            gizmoDragging = false;

            SetCursorLocked(false);
        }
    }

    SceneEditor::Camera& GetSceneCamera()
    {
        return camera;
    }

    void DrawSceneEditor(const char* title, bool* open)
    {
        if (open != nullptr && !*open)
        {
            selecting = false;
            ReleaseMouseCapture();
            return;
        }

        ImGui::Begin(title, open);

        DrawSceneDocumentTabs();

        const ImVec2 available = ImGui::GetContentRegionAvail();

        if (available.x <= 1.0f || available.y <= 1.0f)
        {
            selecting = false;
            ReleaseMouseCapture();

            ImGui::End();
            return;
        }

        const ImVec2 start = ImGui::GetCursorScreenPos();

        const ImVec2 end(
            start.x + available.x,
            start.y + available.y
        );

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        const EnvironmentSettings& env = GetEnvironmentSettings();

        drawList->PushClipRect(start, end, true);

        drawList->AddRectFilled(
            start,
            end,
            ImGui::ColorConvertFloat4ToU32(env.backgroundColor)
        );

        ImGui::InvisibleButton(
            "SceneViewport",
            available,
            ImGuiButtonFlags_MouseButtonLeft |
            ImGuiButtonFlags_MouseButtonMiddle |
            ImGuiButtonFlags_MouseButtonRight
        );

        const bool hovered = ImGui::IsItemHovered();

        const bool focused = ImGui::IsWindowFocused(
            ImGuiFocusedFlags_RootAndChildWindows
        );

        ImGuiIO& io = ImGui::GetIO();

        const ImVec2 mouse = io.MousePos;

        const ImVec2 gizmoCenter(
            end.x - 65.0f,
            start.y + 65.0f
        );

        const float gizmoHitRadius = 64.0f;

        const float gx = mouse.x - gizmoCenter.x;
        const float gy = mouse.y - gizmoCenter.y;

        const bool overGizmo =
            gx * gx + gy * gy <=
            gizmoHitRadius * gizmoHitRadius;

        if (hovered &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            gizmoDragging = overGizmo;
        }

        const bool wantsCameraCapture =
            hovered &&
            (
                ImGui::IsMouseDown(ImGuiMouseButton_Right) ||
                ImGui::IsMouseDown(ImGuiMouseButton_Middle)
                );

        const bool wantsGizmoCapture =
            gizmoDragging &&
            ImGui::IsMouseDown(ImGuiMouseButton_Left);

        if (!mouseCaptureActive &&
            (wantsCameraCapture || wantsGizmoCapture))
        {
            mouseCaptureActive = true;
            SetCursorLocked(true);
        }

        if (mouseCaptureActive)
        {
            SetCursorLocked(true);

            if (!IsAnyMouseButtonDown())
            {
                ReleaseMouseCapture();
            }
        }
        else
        {
            SetCursorLocked(false);
        }

        const bool rightDown =
            mouseCaptureActive &&
            ImGui::IsMouseDown(ImGuiMouseButton_Right);

        const bool middleDown =
            mouseCaptureActive &&
            ImGui::IsMouseDown(ImGuiMouseButton_Middle);

        const bool leftDown =
            ImGui::IsMouseDown(ImGuiMouseButton_Left);

        const bool fpsNavigation = rightDown && !overGizmo;

        if (!mouseCaptureActive &&
            hovered &&
            !overGizmo)
        {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                selecting = true;
                selectionStart = mouse;
                selectionEnd = mouse;
            }

            if (io.MouseWheel != 0.0f)
            {
                camera.Zoom(io.MouseWheel);
            }
        }

        if (selecting && leftDown)
        {
            selectionEnd = mouse;
        }

        if (selecting &&
            ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            selecting = false;
        }

        if (middleDown && !fpsNavigation)
        {
            const ImVec2 delta = io.MouseDelta;

            if (io.KeyShift)
            {
                camera.Pan(delta.x, delta.y);
            }
            else
            {
                camera.Orbit(delta.x, delta.y);
            }
        }

        if (fpsNavigation)
        {
            if (!camera.fpsMode)
            {
                camera.BeginFPS();
            }

            camera.navigating = true;

            camera.RotateFPS(
                io.MouseDelta.x,
                io.MouseDelta.y
            );

            const float forward =
                (ImGui::IsKeyDown(ImGuiKey_W) ? 1.0f : 0.0f) -
                (ImGui::IsKeyDown(ImGuiKey_S) ? 1.0f : 0.0f);

            const float right =
                (ImGui::IsKeyDown(ImGuiKey_D) ? 1.0f : 0.0f) -
                (ImGui::IsKeyDown(ImGuiKey_A) ? 1.0f : 0.0f);

            const float up =
                (ImGui::IsKeyDown(ImGuiKey_E) ? 1.0f : 0.0f) -
                (ImGui::IsKeyDown(ImGuiKey_Q) ? 1.0f : 0.0f);

            camera.MoveFPS(
                forward,
                right,
                up,
                io.DeltaTime
            );
        }
        else
        {
            camera.navigating = false;

            if (camera.fpsMode)
            {
                camera.EndFPS();
            }
        }


        // Render the 3D scene into an offscreen framebuffer.
        const int renderWidth = std::max(
            1,
            static_cast<int>(available.x)
        );

        const int renderHeight = std::max(
            1,
            static_cast<int>(available.y)
        );

        const GLuint sceneTexture =
            SceneViewportRenderer::Get().Render(
                camera,
                renderWidth,
                renderHeight,
                env.backgroundColor
            );

        // Display the framebuffer texture behind the grid
        // and the existing editor overlays.
        if (sceneTexture != 0)
        {
            drawList->AddImage(
                (ImTextureID)(intptr_t)sceneTexture,
                start,
                end,
                ImVec2(0.0f, 1.0f),
                ImVec2(1.0f, 0.0f)
            );
        }


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
                env.showAxes,
                camera
            );
        }
        else if (env.showAxes)
        {
            DrawLine3D(
                drawList,
                { -100.0f, 0.0f, 0.0f },
                { 100.0f, 0.0f, 0.0f },
                sceneCenter,
                available.x,
                available.y,
                IM_COL32(245, 75, 75, 255),
                camera,
                2.0f
            );

            DrawLine3D(
                drawList,
                { 0.0f, -100.0f, 0.0f },
                { 0.0f, 100.0f, 0.0f },
                sceneCenter,
                available.x,
                available.y,
                IM_COL32(80, 225, 120, 255),
                camera,
                2.0f
            );

            DrawLine3D(
                drawList,
                { 0.0f, 0.0f, -100.0f },
                { 0.0f, 0.0f, 100.0f },
                sceneCenter,
                available.x,
                available.y,
                IM_COL32(80, 165, 255, 255),
                camera,
                2.0f
            );
        }

        DrawViewGizmo(
            drawList,
            gizmoCenter,
            camera,
            hovered && overGizmo
        );

        drawList->AddText(
            ImVec2(start.x + 12.0f, start.y + 10.0f),
            IM_COL32(225, 230, 240, 230),
            "Scene View"
        );

        if (camera.navigating)
        {
            drawList->AddText(
                ImVec2(start.x + 12.0f, start.y + 32.0f),
                IM_COL32(180, 190, 210, 220),
                "RMB: Look | WASD: Move | Q/E: Down/Up | Shift: Faster"
            );
        }

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
                boxMin,
                boxMax,
                IM_COL32(80, 150, 255, 45)
            );

            drawList->AddRect(
                boxMin,
                boxMax,
                IM_COL32(100, 175, 255, 240)
            );
        }

        drawList->PopClipRect();

        ImGui::End();
    }
}