
#include "tab/windows/scene_editor/scene_editor.h"

#include "imgui.h"

namespace CNode
{
    void DrawSceneEditor(const char* title, bool* open)
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
