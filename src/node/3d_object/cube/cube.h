
#pragma once

#include <glm/glm.hpp>

#include "tab/windows/node_graph/node_graph_data.h"

namespace CNode
{
    struct CubeObject
    {
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

        bool visible = true;
    };

    // Membuat data Cube. Tidak otomatis memasukkannya ke scene.
    CubeObject CreateCubeObject(
        const glm::vec3& position
    );

    // Menggambar Cube menggunakan matriks ViewProjection.
    void DrawCubeObject(
        const CubeObject& cube,
        const glm::mat4& viewProjection
    );

    // Menambahkan node Cube beserta class default ke Node Graph.
    NodeID AddCubeNode(
        NodeGraph& graph,
        float x = 0.0f,
        float y = 0.0f
    );
}
