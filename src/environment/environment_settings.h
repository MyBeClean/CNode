
#pragma once

#include "imgui.h"

namespace CNode
{
    struct EnvironmentSettings
    {
        ImVec4 backgroundColor = ImVec4(
            0.12f, 0.14f, 0.18f, 1.0f
        );

        ImVec4 ambientColor = ImVec4(
            0.35f, 0.38f, 0.45f, 1.0f
        );

        float ambientIntensity = 1.0f;

        bool useSkybox = false;
        bool showGrid = true;
        bool showAxes = true;
    };

    EnvironmentSettings& GetEnvironmentSettings();
}
