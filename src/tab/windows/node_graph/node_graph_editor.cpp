#include "node_graph_editor.h"
#include "imgui.h"

namespace CNode
{
    void DrawNodeGraphEditor(const char* title, bool* open)
    {
        ImGui::Begin(title, open);
        ImGui::End();
    }
}