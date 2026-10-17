#include "tab/windows/project_setting/project_setting.h"

#include "imgui.h"

namespace CNode
{
    void DrawProjectSetting(const char* title, bool* open)
    {
        if (!ImGui::Begin(title, open))
        {
            ImGui::End();
            return;
        }

        // Kosong untuk pengujian sistem multi-window.
        ImGui::End();
    }
}
