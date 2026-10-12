#include "inspector.h"
#include "imgui.h"

namespace CNode
{
    void DrawInspector(const char* title, bool* open)
    {
        ImGui::Begin(title, open);
        ImGui::End();
    }
}