
#pragma once

#include "registration/registration_data.h"
#include "tab/windows/scene_editor/scene_editor.h"

namespace CNode
{
    namespace
    {
        const bool sceneEditorAttributeRegistered =
            RegisterWindowAttribute(
                "scene_editor",
                "Scene Editor",
                &DrawSceneEditor,
                ImGuiDir_None
            );
    }
}
