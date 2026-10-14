#include "node/3d_object/cube/cube_render.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>

namespace CNode
{
    namespace
    {
        // Setiap vertex menyimpan:
        // Position XYZ + Normal XYZ.
        constexpr float CubeVertices[] =
        {
            // Front
            -0.5f,-0.5f, 0.5f,  0, 0, 1,
             0.5f,-0.5f, 0.5f,  0, 0, 1,
             0.5f, 0.5f, 0.5f,  0, 0, 1,

             0.5f, 0.5f, 0.5f,  0, 0, 1,
            -0.5f, 0.5f, 0.5f,  0, 0, 1,
            -0.5f,-0.5f, 0.5f,  0, 0, 1,

            // Back
            -0.5f,-0.5f,-0.5f,  0, 0,-1,
             0.5f, 0.5f,-0.5f,  0, 0,-1,
             0.5f,-0.5f,-0.5f,  0, 0,-1,

             0.5f, 0.5f,-0.5f,  0, 0,-1,
            -0.5f,-0.5f,-0.5f,  0, 0,-1,
            -0.5f, 0.5f,-0.5f,  0, 0,-1,

            // Left
            -0.5f,-0.5f,-0.5f, -1, 0, 0,
            -0.5f,-0.5f, 0.5f, -1, 0, 0,
            -0.5f, 0.5f, 0.5f, -1, 0, 0,

            -0.5f, 0.5f, 0.5f, -1, 0, 0,
            -0.5f, 0.5f,-0.5f, -1, 0, 0,
            -0.5f,-0.5f,-0.5f, -1, 0, 0,

            // Right
             0.5f,-0.5f,-0.5f,  1, 0, 0,
             0.5f, 0.5f, 0.5f,  1, 0, 0,
             0.5f,-0.5f, 0.5f,  1, 0, 0,

             0.5f, 0.5f, 0.5f,  1, 0, 0,
             0.5f,-0.5f,-0.5f,  1, 0, 0,
             0.5f, 0.5f,-0.5f,  1, 0, 0,

             // Top
             -0.5f, 0.5f,-0.5f,  0, 1, 0,
             -0.5f, 0.5f, 0.5f,  0, 1, 0,
              0.5f, 0.5f, 0.5f,  0, 1, 0,

              0.5f, 0.5f, 0.5f,  0, 1, 0,
              0.5f, 0.5f,-0.5f,  0, 1, 0,
             -0.5f, 0.5f,-0.5f,  0, 1, 0,

             // Bottom
             -0.5f,-0.5f,-0.5f,  0,-1, 0,
              0.5f,-0.5f, 0.5f,  0,-1, 0,
             -0.5f,-0.5f, 0.5f,  0,-1, 0,

              0.5f,-0.5f, 0.5f,  0,-1, 0,
             -0.5f,-0.5f,-0.5f,  0,-1, 0,
              0.5f,-0.5f,-0.5f,  0,-1, 0
        };

        const char* VertexShaderSource = R"(
            #version 330 core

            layout (location = 0) in vec3 aPosition;
            layout (location = 1) in vec3 aNormal;

            uniform mat4 uModel;
            uniform mat4 uViewProjection;

            out vec3 vNormal;

            void main()
            {
                vec4 worldPosition = uModel * vec4(aPosition, 1.0);

                gl_Position =
                    uViewProjection * worldPosition;

                vNormal = mat3(transpose(inverse(uModel)))
                    * aNormal;
            }
        )";

        const char* FragmentShaderSource = R"(
            #version 330 core

            in vec3 vNormal;

            out vec4 FragColor;

            void main()
            {
                vec3 normal = normalize(vNormal);
                vec3 lightDirection =
                    normalize(vec3(-0.4, 0.8, 0.6));

                float diffuse =
                    max(dot(normal, lightDirection), 0.0);

                float lighting = 0.25 + diffuse * 0.75;

                vec3 baseColor = vec3(0.72, 0.75, 0.80);

                FragColor = vec4(
                    baseColor * lighting,
                    1.0
                );
            }
        )";

        GLuint CompileShader(
            GLenum shaderType,
            const char* source)
        {
            GLuint shader = glCreateShader(shaderType);

            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);

            GLint success = GL_FALSE;

            glGetShaderiv(
                shader,
                GL_COMPILE_STATUS,
                &success
            );

            if (!success)
            {
                char infoLog[1024] = {};

                glGetShaderInfoLog(
                    shader,
                    sizeof(infoLog),
                    nullptr,
                    infoLog
                );

                std::cerr
                    << "Cube shader compilation failed: "
                    << infoLog
                    << '\n';

                glDeleteShader(shader);

                return 0;
            }

            return shader;
        }
    }

    CubeRenderer& CubeRenderer::Get()
    {
        static CubeRenderer instance;
        return instance;
    }

    bool CubeRenderer::CreateShaderProgram()
    {
        const GLuint vertexShader =
            CompileShader(
                GL_VERTEX_SHADER,
                VertexShaderSource
            );

        if (vertexShader == 0)
            return false;

        const GLuint fragmentShader =
            CompileShader(
                GL_FRAGMENT_SHADER,
                FragmentShaderSource
            );

        if (fragmentShader == 0)
        {
            glDeleteShader(vertexShader);
            return false;
        }

        shaderProgram = glCreateProgram();

        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);

        glLinkProgram(shaderProgram);

        GLint success = GL_FALSE;

        glGetProgramiv(
            shaderProgram,
            GL_LINK_STATUS,
            &success
        );

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        if (!success)
        {
            char infoLog[1024] = {};

            glGetProgramInfoLog(
                shaderProgram,
                sizeof(infoLog),
                nullptr,
                infoLog
            );

            std::cerr
                << "Cube shader linking failed: "
                << infoLog
                << '\n';

            glDeleteProgram(shaderProgram);
            shaderProgram = 0;

            return false;
        }

        return true;
    }

    bool CubeRenderer::Initialize()
    {
        if (initialized)
            return true;

        if (!CreateShaderProgram())
            return false;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(CubeVertices),
            CubeVertices,
            GL_STATIC_DRAW
        );

        // Position.
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            6 * sizeof(float),
            nullptr
        );

        glEnableVertexAttribArray(0);

        // Normal.
        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            6 * sizeof(float),
            reinterpret_cast<void*>(
                3 * sizeof(float)
                )
        );

        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        initialized = true;

        return true;
    }

    void CubeRenderer::Draw(
        const glm::mat4& model,
        const glm::mat4& viewProjection)
    {
        if (!initialized)
            return;

        glUseProgram(shaderProgram);

        const GLint modelLocation =
            glGetUniformLocation(
                shaderProgram,
                "uModel"
            );

        const GLint viewProjectionLocation =
            glGetUniformLocation(
                shaderProgram,
                "uViewProjection"
            );

        glUniformMatrix4fv(
            modelLocation,
            1,
            GL_FALSE,
            glm::value_ptr(model)
        );

        glUniformMatrix4fv(
            viewProjectionLocation,
            1,
            GL_FALSE,
            glm::value_ptr(viewProjection)
        );

        glBindVertexArray(vao);

        glDrawArrays(
            GL_TRIANGLES,
            0,
            36
        );

        glBindVertexArray(0);

        glUseProgram(0);
    }

    void CubeRenderer::Shutdown()
    {
        if (vao != 0)
        {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }

        if (vbo != 0)
        {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }

        if (shaderProgram != 0)
        {
            glDeleteProgram(shaderProgram);
            shaderProgram = 0;
        }

        initialized = false;
    }
}