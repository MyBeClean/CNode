
#include "class/transform/transform.h"

#include "class/class_registry.h"

#include "class/transform/variables/position.h"
#include "class/transform/variables/rotation.h"
#include "class/transform/variables/scale.h"

namespace CNode
{
    bool RegisterTransformClass()
    {
        ClassDefinition definition;

        definition.name = "Transform";
        definition.category = "Transform";

        // Transform hanya boleh muncul sekali
        // pada setiap node.
        definition.canDuplicate = false;

        definition.hasVariables = true;

        definition.initialize =
            [](ClassInstance& instance)
            {
                // Setiap variabel dikelola oleh header sendiri.
                instance.variables.push_back(
                    CreatePositionVariable()
                );

                instance.variables.push_back(
                    CreateRotationVariable()
                );

                instance.variables.push_back(
                    CreateScaleVariable()
                );
            };

        return RegisterClass(definition);
    }
}
