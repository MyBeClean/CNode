
#include "class/visual/visual.h"

#include "class/class_registry.h"

#include "class/visual/variables/visibility.h"
#include "class/visual/variables/material.h"

namespace CNode
{
    bool RegisterVisualClass()
    {
        ClassDefinition definition;

        definition.name = "Visual";
        definition.category = "Visual";

        // Visual hanya boleh muncul sekali pada node.
        definition.canDuplicate = false;

        definition.hasVariables = true;

        definition.initialize =
            [](ClassInstance& instance)
            {
                instance.variables.push_back(
                    CreateVisibilityVariable()
                );

                instance.variables.push_back(
                    CreateMaterialVariable()
                );
            };

        return RegisterClass(definition);
    }
}
