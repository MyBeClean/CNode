#include "registration/registration_data.h"
#include "tab/windows/node_graph/node_graph.h"

namespace CNode
{
    namespace
    {
        const bool nodeGraphAttributeRegistered =
            RegisterWindowAttribute(
                "node_graph",
                "Node Graph",
                &DrawNodeGraph,
                ImGuiDir_None
            );
    }
}