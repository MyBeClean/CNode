#include "scene/window/font.h"

namespace CNode
{
    bool EditorFont::Load()
    {
        ImGuiIO& io = ImGui::GetIO();

        font = io.Fonts->AddFontFromFileTTF(
            "assets/fonts/Inter.ttf",
            16.0f
        );

        if (font == nullptr)
        {
            io.Fonts->AddFontDefault();
            return false;
        }

        return true;
    }

    void EditorFont::Apply() const
    {
        if (font != nullptr)
        {
            ImGui::PushFont(font);
        }
    }

    void EditorFont::Reset()
    {
        font = nullptr;
    }
}