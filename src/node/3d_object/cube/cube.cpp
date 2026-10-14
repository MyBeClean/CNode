#include "class/transform/transform.h"
#include "class/visual/visual.h"

#include "node/3d_object/cube/cube.h"
#include "node/3d_object/cube/cube_mapping.h"
#include "node/3d_object/cube/cube_render.h"

namespace CNode
{
    CubeObject CreateCubeObject(
        const glm::vec3& position)
    {
        CubeObject cube;

        cube.position = position;
        cube.rotation = glm::vec3(0.0f);
        cube.scale = glm::vec3(1.0f);
        cube.visible = true;

        return cube;
    }

    void DrawCubeObject(
        const CubeObject& cube,
        const glm::mat4& viewProjection)
    {
        if (!cube.visible)
            return;

        const glm::mat4 model =
            BuildCubeModelMatrix(
                cube.position,
                cube.rotation,
                cube.scale
            );

        CubeRenderer::Get().Draw(
            model,
            viewProjection
        );
    }

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
        node.width = 260.0f;
        node.height = 180.0f;
        node.selected = false;

        const NodeID id = node.id;

        graph.nodes.push_back(node);

        return id;
    }
}