
#pragma once

#include "tab/windows/node_graph/node_graph_data.h"

namespace CNode
{
    inline ClassVariable CreateScaleVariable()
    {
        ClassVariable variable;

        variable.name = "Scale";
        variable.type = SocketType::Vector3;

        variable.control = VariableControl::InputField;

        variable.hasInputSocket = true;
        variable.hasOutputSocket = false;

        variable.vectorValue = {
            1.0f, 1.0f, 1.0f
        };

        return variable;
    }
}
