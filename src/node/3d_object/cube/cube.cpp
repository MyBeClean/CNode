
#include "node/3d_object/cube/cube.h"
#include "node/3d_object/cube/cube_mapping.h"
#include "node/3d_object/cube/cube_render.h"
#include "node/3d_object/cube/cube_spawn.h"

#include "class/class_registry.h"
#include <utility>

#include <algorithm>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/geometric.hpp>

namespace CNode
{
    namespace
    {
        // =====================================================
        // CUBE GEOMETRY
        // Position XYZ + Normal XYZ per vertex.
        // =====================================================

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

        // =====================================================
        // SHADERS
        // =====================================================

        constexpr const char* VertexShaderSource = R"(
            #version 330 core

            layout (location = 0) in vec3 aPosition;
            layout (location = 1) in vec3 aNormal;

            uniform mat4 uModel;
            uniform mat4 uViewProjection;

            out vec3 vNormal;

            void main()
            {
                vec4 worldPosition =
                    uModel * vec4(aPosition, 1.0);

                gl_Position =
                    uViewProjection * worldPosition;

                vNormal =
                    mat3(transpose(inverse(uModel))) * aNormal;
            }
        )";

        constexpr const char* FragmentShaderSource = R"(
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

        // =====================================================
        // SHADER COMPILATION
        // =====================================================

        GLuint CompileShader(
            GLenum shaderType,
            const char* source)
        {
            const GLuint shader = glCreateShader(shaderType);

            if (shader == 0)
            {
                std::cerr << "Failed to create Cube shader.\n";
                return 0;
            }

            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);

            GLint success = GL_FALSE;

            glGetShaderiv(
                shader,
                GL_COMPILE_STATUS,
                &success
            );

            if (success == GL_FALSE)
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
                    << infoLog << '\n';

                glDeleteShader(shader);
                return 0;
            }

            return shader;
        }

        // =====================================================
        // DEFAULT NODE CLASSES
        // =====================================================

        ClassInstance CreateDefaultCubeClass(
            NodeGraph& graph,
            const char* className)
        {
            ClassInstance classInstance;

            classInstance.id = graph.nextClassID++;
            classInstance.className = className;
            classInstance.canDuplicate = true;
            classInstance.hasVariables = true;
            classInstance.expanded = true;

            return classInstance;
        }
    }

    // =========================================================
    // CUBE OBJECT
    // =========================================================

    CubeObject CreateCubeObject(
        const glm::vec3& position)
    {
        CubeObject cube;

        cube.position = position;
        cube.rotation = glm::vec3(0.0f);
        cube.scale = glm::vec3(1.0f);
        cube.visible = true;

        // Penting:
        // Fungsi ini hanya membuat data Cube.
        // Fungsi ini TIDAK mendaftarkan Cube ke scene.
        // Pemanggil bertanggung jawab atas penyimpanan objek.

        return cube;
    }

    void DrawCubeObject(
        const CubeObject& cube,
        const glm::mat4& viewProjection)
    {
        if (!cube.visible)
            return;

        CubeRenderer& renderer = CubeRenderer::Get();

        if (!renderer.IsInitialized())
            return;

        const glm::mat4 model = BuildCubeModelMatrix(
            cube.position,
            cube.rotation,
            cube.scale
        );

        renderer.Draw(model, viewProjection);
    }

    // =========================================================
    // NODE GRAPH
    // =========================================================

    // =========================================================
// NODE GRAPH
// =========================================================

    NodeID AddCubeNode(
        NodeGraph& graph,
        float x,
        float y)
    {
        GraphNode node;

        node.id = graph.nextNodeID++;
        node.name = "Cube";
        node.x = x;
        node.y = y;
        node.width = 280.0f;
        node.height = 220.0f;
        node.selected = false;

        const NodeID id = node.id;

        graph.nodes.push_back(std::move(node));

        // Gunakan registry agar variabel default dibuat
        // sesuai definisi Transform dan Visual.
        const AddClassResult transformResult =
            AddClassToNode(graph, id, "Transform");

        const AddClassResult visualResult =
            AddClassToNode(graph, id, "Visual");

        if (transformResult != AddClassResult::Success ||
            visualResult != AddClassResult::Success)
        {
            // Jangan meninggalkan node yang setengah jadi.
            graph.nodes.erase(
                std::remove_if(
                    graph.nodes.begin(),
                    graph.nodes.end(),
                    [id](const GraphNode& item)
                    {
                        return item.id == id;
                    }),
                graph.nodes.end()
            );

            return 0;
        }

        return id;
    }

    // =========================================================
    // TRANSFORM / MAPPING
    // =========================================================

    glm::mat4 BuildCubeModelMatrix(
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale)
    {
        glm::mat4 model(1.0f);

        model = glm::translate(model, position);

        model = glm::rotate(
            model,
            glm::radians(rotation.x),
            glm::vec3(1.0f, 0.0f, 0.0f)
        );

        model = glm::rotate(
            model,
            glm::radians(rotation.y),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        model = glm::rotate(
            model,
            glm::radians(rotation.z),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );

        model = glm::scale(model, scale);

        return model;
    }

    // =========================================================
    // CUBE SPAWN
    // =========================================================

    glm::vec3 CalculateCubeSpawnPosition(
        const glm::vec3& cameraPosition,
        const glm::vec3& cameraForward,
        const CubeSpawnSettings& settings)
    {
        const float forwardLength =
            glm::length(cameraForward);

        if (forwardLength < 0.0001f)
        {
            return cameraPosition +
                glm::vec3(
                    0.0f,
                    0.0f,
                    -std::max(settings.distance, 0.1f)
                );
        }

        const glm::vec3 forward =
            cameraForward / forwardLength;

        const float distance =
            std::max(settings.distance, 0.1f);

        return cameraPosition + forward * distance;
    }

    // =========================================================
    // CUBE RENDERER
    // =========================================================

    CubeRenderer& CubeRenderer::Get()
    {
        static CubeRenderer instance;
        return instance;
    }

    bool CubeRenderer::CreateShaderProgram()
    {
        const GLuint vertexShader = CompileShader(
            GL_VERTEX_SHADER,
            VertexShaderSource
        );

        if (vertexShader == 0)
            return false;

        const GLuint fragmentShader = CompileShader(
            GL_FRAGMENT_SHADER,
            FragmentShaderSource
        );

        if (fragmentShader == 0)
        {
            glDeleteShader(vertexShader);
            return false;
        }

        shaderProgram = glCreateProgram();

        if (shaderProgram == 0)
        {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return false;
        }

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

        if (success == GL_FALSE)
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
                << infoLog << '\n';

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

        // Panggil fungsi ini setelah konteks OpenGL aktif
        // dan GLAD berhasil dimuat.
        if (!CreateShaderProgram())
            return false;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        if (vao == 0 || vbo == 0)
        {
            Shutdown();
            return false;
        }

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(CubeVertices),
            CubeVertices,
            GL_STATIC_DRAW
        );

        // Vertex position: XYZ.
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            6 * sizeof(float),
            nullptr
        );

        glEnableVertexAttribArray(0);

        // Vertex normal: XYZ.
        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            6 * sizeof(float),
            reinterpret_cast<void*>(3 * sizeof(float))
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

        const GLint modelLocation = glGetUniformLocation(
            shaderProgram,
            "uModel"
        );

        const GLint viewProjectionLocation = glGetUniformLocation(
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
        // Harus dipanggil saat konteks OpenGL masih aktif.
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
