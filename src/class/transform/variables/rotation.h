
#pragma once

#include "tab/windows/node_graph/node_graph_data.h"

namespace CNode
{
    inline ClassVariable CreateRotationVariable()
    {
        ClassVariable variable;

        variable.name = "Rotation";
        variable.type = SocketType::Vector3;

        variable.control = VariableControl::InputField;

        variable.hasInputSocket = true;
        variable.hasOutputSocket = false;

        variable.vectorValue = {
            0.0f, 0.0f, 0.0f
        };

        return variable;
    }
}
