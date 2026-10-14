
#pragma once

#include <glad/glad.h>
#include <imgui.h>

#include "scene_camera.h"

namespace CNode
{
    class SceneViewportRenderer
    {
    public:
        static SceneViewportRenderer& Get();

        bool Initialize();

        GLuint Render(
            const SceneEditor::Camera& camera,
            int width,
            int height,
            const ImVec4& backgroundColor
        );

        void Shutdown();

        GLuint GetTexture() const
        {
            return colorTexture;
        }

    private:
        SceneViewportRenderer() = default;

        SceneViewportRenderer(
            const SceneViewportRenderer&
        ) = delete;

        SceneViewportRenderer& operator=(
            const SceneViewportRenderer&
            ) = delete;

        bool Resize(int width, int height);

        GLuint framebuffer = 0;
        GLuint colorTexture = 0;
        GLuint depthBuffer = 0;

        int viewportWidth = 0;
        int viewportHeight = 0;

        bool initialized = false;
    };
}
