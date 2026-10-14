#include "object_list.h"

#include "imgui.h"

#include "object_list_render.h"

namespace CNode
{
    void DrawObjectList(const char* title, bool* open)
    {
        if (!ImGui::Begin(title, open))
        {
            ImGui::End();
            return;
        }

        DrawObjectListContents();

        ImGui::End();
    }
}