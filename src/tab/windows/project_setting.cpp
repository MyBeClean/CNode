
#include "project_setting.h"
#include "environment/environment_settings.h"

#include "imgui.h"

namespace CNode
{
    void DrawProjectSettings(
        const char* title,
        bool* open
    )
    {
        ImGui::Begin(title, open);

        if (ImGui::BeginTabBar("ProjectSettingsTabs"))
        {
            if (ImGui::BeginTabItem("Environment"))
            {
                EnvironmentSettings& env =
                    GetEnvironmentSettings();

                ImGui::Spacing();
                ImGui::TextUnformatted("Environment Settings");
                ImGui::Separator();

                ImGui::ColorEdit4(
                    "Background Color",
                    &env.backgroundColor.x
                );

                ImGui::ColorEdit4(
                    "Ambient Color",
                    &env.ambientColor.x
                );

                ImGui::SliderFloat(
                    "Ambient Intensity",
                    &env.ambientIntensity,
                    0.0f,
                    5.0f
                );

                ImGui::Checkbox(
                    "Use Skybox",
                    &env.useSkybox
                );

                ImGui::Separator();
                ImGui::TextUnformatted("Scene View");

                ImGui::Checkbox(
                    "Show Grid",
                    &env.showGrid
                );

                ImGui::Checkbox(
                    "Show XYZ Axes",
                    &env.showAxes
                );

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
}
