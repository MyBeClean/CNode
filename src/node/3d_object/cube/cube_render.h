#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace CNode
{
    class CubeRenderer
    {
    public:
        static CubeRenderer& Get();

        bool Initialize();

        void Draw(
            const glm::mat4& model,
            const glm::mat4& viewProjection
        );

        void Shutdown();

        bool IsInitialized() const
        {
            return initialized;
        }

    private:
        CubeRenderer() = default;

        ~CubeRenderer() = default;

        CubeRenderer(const CubeRenderer&) = delete;
        CubeRenderer& operator=(const CubeRenderer&) = delete;

        bool CreateShaderProgram();

        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint shaderProgram = 0;

        bool initialized = false;
    };
}