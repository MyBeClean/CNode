#pragma once

#include <glm/glm.hpp>

namespace CNode
{
    struct CubeSpawnSettings
    {
        float distance = 5.0f;
    };

    glm::vec3 CalculateCubeSpawnPosition(
        const glm::vec3& cameraPosition,
        const glm::vec3& cameraForward,
        const CubeSpawnSettings& settings = {}
    );
}