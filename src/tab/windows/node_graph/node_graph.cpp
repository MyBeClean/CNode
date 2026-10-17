#include "tab/windows/node_graph/node_graph.h"

#include "imgui.h"

namespace CNode
{
    void DrawNodeGraph(const char* title, bool* open)
    {
        if (!ImGui::Begin(title, open))
        {
            ImGui::End();
            return;
        }

        // Node Graph sengaja dikosongkan untuk pengujian registrasi.

        ImGui::End();
    }
}