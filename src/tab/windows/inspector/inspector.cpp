
#include "tab/windows/inspector/inspector.h"

#include "imgui.h"

namespace CNode
{
    void DrawInspector(const char* title, bool* open)
    {
        if (!ImGui::Begin(title, open))
        {
            ImGui::End();
            return;
        }

        // Inspector sengaja dikosongkan untuk pengujian registrasi.

        ImGui::End();
    }
}
