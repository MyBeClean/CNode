
#include "registration/registration.h"
#include "registration/registration_data.h"

#include "imgui.h"

namespace CNode
{
    void DrawRegisteredWindows()
    {
        auto& windows = GetRegisteredWindows();

        // Menu utama aplikasi.
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Windows"))
            {
                for (WindowAttribute& window : windows)
                {
                    if (ImGui::MenuItem(
                        window.name,
                        nullptr,
                        window.open))
                    {
                        window.open = !window.open;
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // Gambar hanya window yang sedang terbuka.
        for (WindowAttribute& window : windows)
        {
            if (window.open && window.draw)
            {
                window.draw(&window.open);
            }
        }
    }
}
