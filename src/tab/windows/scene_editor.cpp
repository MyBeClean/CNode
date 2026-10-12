#include "scene_editor.h"
#include "imgui.h"

namespace CNode
{
    void DrawSceneEditor(const char* title, bool* open)
    {
        ImGui::Begin(title, open);
        ImGui::End();
    }
}