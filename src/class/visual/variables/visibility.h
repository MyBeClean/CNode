
#pragma once

#include "tab/windows/node_graph/node_graph_data.h"

namespace CNode
{
    inline ClassVariable CreateVisibilityVariable()
    {
        ClassVariable variable;

        variable.name = "Visibility";
        variable.type = SocketType::Boolean;

        variable.control = VariableControl::Checkbox;

        variable.hasInputSocket = false;
        variable.hasOutputSocket = false;

        // Secara default, mesh terlihat.
        variable.boolValue = true;

        return variable;
    }
}
