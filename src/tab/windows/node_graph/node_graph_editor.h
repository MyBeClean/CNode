#pragma once

#include "tab/windows/node_graph/node_graph_data.h"

namespace CNode
{
    void DrawNodeGraphEditor(
        const char* title,
        bool* open
    );

    NodeGraph& GetActiveNodeGraph();
    void ResetNodeGraph();
}