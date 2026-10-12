
#include "file_tab.h"
#include "new_scene.h"
#include "open_scene.h"
#include "imgui.h"

namespace CNode
{
    void DrawFileTab()
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene"))
                NewScene();

            if (ImGui::MenuItem("Open Scene"))
                OpenScene();

            ImGui::EndMenu();
        }
    }
}
