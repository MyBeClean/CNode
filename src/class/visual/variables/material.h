
#pragma once

#include "tab/windows/node_graph/node_graph_data.h"

namespace CNode
{
    inline ClassVariable CreateMaterialVariable()
    {
        ClassVariable variable;

        variable.name = "Material";
        variable.type = SocketType::Material;

        variable.control = VariableControl::Slot;

        variable.hasInputSocket = false;
        variable.hasOutputSocket = false;

        // Kosong berarti belum ada material yang dipilih.
        variable.selectedResource.clear();

        return variable;
    }
}
