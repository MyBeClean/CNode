#pragma once

#include "tab/windows/node_graph/node_graph_data.h"

#include <glm/glm.hpp>

namespace CNode
{
    struct CubeObject
    {
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

        bool visible = true;
    };

    // Membuat data objek Cube.
    CubeObject CreateCubeObject(
        const glm::vec3& position
    );

    // Menggambar Cube menggunakan kamera Scene View.
    void DrawCubeObject(
        const CubeObject& cube,
        const glm::mat4& viewProjection
    );

    // Membuat node Cube di dalam Node Graph.
    NodeID AddCubeNode(
        NodeGraph& graph,
        float x = 0.0f,
        float y = 0.0f
    );
}