
#include "tab/windows/scene_editor/scene_editor.h"

#include "imgui.h"

namespace CNode
{
    void DrawSceneEditor(bool* open)
    {
        if (!ImGui::Begin("Scene Editor", open))
        {
            ImGui::End();
            return;
        }

        // Scene Editor sengaja dikosongkan untuk pengujian.
        ImGui::End();
    }
}
