
#pragma once

#include "registration/registration_data.h"
#include "tab/windows/inspector/inspector.h"

namespace CNode
{
    namespace
    {
        const bool inspectorAttributeRegistered =
            RegisterWindowAttribute(
                "inspector",
                "Inspector",
                &DrawInspector,
                ImGuiDir_None
            );
    }
}
