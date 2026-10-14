
#include "scene_viewport_renderer.h"

#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "node/3d_object/cube/cube_render.h"

namespace CNode
{
    SceneViewportRenderer& SceneViewportRenderer::Get()
    {
        static SceneViewportRenderer instance;
        return instance;
    }

    bool SceneViewportRenderer::Initialize()
    {
        if (initialized)
            return true;

        glGenFramebuffers(1, &framebuffer);
        glGenTextures(1, &colorTexture);
        glGenRenderbuffers(1, &depthBuffer);

        initialized =
            framebuffer != 0 &&
            colorTexture != 0 &&
            depthBuffer != 0;

        if (!initialized)
            Shutdown();

        return initialized;
    }

    bool SceneViewportRenderer::Resize(int width, int height)
    {
        width = std::max(width, 1);
        height = std::max(height, 1);

        if (width == viewportWidth && height == viewportHeight)
            return true;

        GLint previousFramebuffer = 0;
        GLint previousTexture = 0;
        GLint previousRenderbuffer = 0;

        glGetIntegerv(
            GL_FRAMEBUFFER_BINDING, &previousFramebuffer
        );
        glGetIntegerv(
            GL_TEXTURE_BINDING_2D, &previousTexture
        );
        glGetIntegerv(
            GL_RENDERBUFFER_BINDING, &previousRenderbuffer
        );

        glBindTexture(GL_TEXTURE_2D, colorTexture);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );

        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR
        );
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR
        );
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE
        );
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE
        );

        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);

        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH24_STENCIL8,
            width,
            height
        );

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            colorTexture,
            0
        );

        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER,
            depthBuffer
        );

        const bool complete =
            glCheckFramebufferStatus(GL_FRAMEBUFFER) ==
            GL_FRAMEBUFFER_COMPLETE;

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            static_cast<GLuint>(previousFramebuffer)
        );
        glBindTexture(
            GL_TEXTURE_2D,
            static_cast<GLuint>(previousTexture)
        );
        glBindRenderbuffer(
            GL_RENDERBUFFER,
            static_cast<GLuint>(previousRenderbuffer)
        );

        if (!complete)
            return false;

        viewportWidth = width;
        viewportHeight = height;

        return true;
    }

    GLuint SceneViewportRenderer::Render(
        const SceneEditor::Camera& camera,
        int width,
        int height,
        const ImVec4& backgroundColor
    )
    {
        if (!initialized || width <= 0 || height <= 0)
            return 0;

        if (!Resize(width, height))
            return 0;

        GLint previousFramebuffer = 0;
        GLint previousViewport[4] = {};
        const GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);

        glGetIntegerv(
            GL_FRAMEBUFFER_BINDING, &previousFramebuffer
        );
        glGetIntegerv(GL_VIEWPORT, previousViewport);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, width, height);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glClearColor(
            backgroundColor.x,
            backgroundColor.y,
            backgroundColor.z,
            backgroundColor.w
        );

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT
        );

        const SceneEditor::CameraBasis basis = camera.Basis();
        const SceneEditor::Vec3 eye = camera.Position();

        const glm::vec3 position(eye.x, eye.y, eye.z);

        const glm::vec3 forward(
            basis.forward.x,
            basis.forward.y,
            basis.forward.z
        );

        const glm::vec3 up(
            basis.up.x,
            basis.up.y,
            basis.up.z
        );

        const glm::mat4 view = glm::lookAt(
            position,
            position + forward,
            up
        );

        const float aspect =
            static_cast<float>(width) /
            static_cast<float>(height);

        const glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            aspect,
            0.1f,
            1000.0f
        );

        const glm::mat4 viewProjection = projection * view;
        const glm::mat4 model(1.0f);

        CubeRenderer::Get().Draw(model, viewProjection);

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            static_cast<GLuint>(previousFramebuffer)
        );

        glViewport(
            previousViewport[0],
            previousViewport[1],
            previousViewport[2],
            previousViewport[3]
        );

        if (!depthWasEnabled)
            glDisable(GL_DEPTH_TEST);

        return colorTexture;
    }

    void SceneViewportRenderer::Shutdown()
    {
        if (depthBuffer)
            glDeleteRenderbuffers(1, &depthBuffer);

        if (colorTexture)
            glDeleteTextures(1, &colorTexture);

        if (framebuffer)
            glDeleteFramebuffers(1, &framebuffer);

        framebuffer = 0;
        colorTexture = 0;
        depthBuffer = 0;

        viewportWidth = 0;
        viewportHeight = 0;

        initialized = false;
    }
}
