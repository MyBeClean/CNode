#include "node/3d_object/cube/cube_mapping.h"

#include <glm/gtc/matrix_transform.hpp>

namespace CNode
{
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
}