#pragma once

#include <glm/glm.hpp>

namespace CNode
{
    glm::mat4 BuildCubeModelMatrix(
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale
    );
}