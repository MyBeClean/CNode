#include "node/3d_object/cube/cube_spawn.h"

#include <glm/geometric.hpp>

namespace CNode
{
    glm::vec3 CalculateCubeSpawnPosition(
        const glm::vec3& cameraPosition,
        const glm::vec3& cameraForward,
        const CubeSpawnSettings& settings)
    {
        const float forwardLength =
            glm::length(cameraForward);

        // Jika arah kamera tidak valid, gunakan arah depan default.
        if (forwardLength < 0.0001f)
        {
            return cameraPosition +
                glm::vec3(0.0f, 0.0f, -settings.distance);
        }

        const glm::vec3 forward =
            cameraForward / forwardLength;

        const float distance =
            settings.distance > 0.1f
            ? settings.distance
            : 0.1f;

        return cameraPosition + forward * distance;
    }
}