#pragma once

#include "registration/registration_data.h"
#include "tab/windows/scene_editor/scene_editor.h"

namespace CNode
{
    namespace
    {
        const bool sceneEditorRegistered = RegisterWindow(
            {
                "scene_editor",
                "Scene Editor",
                true,
                &DrawSceneEditor
            }
        );
    }
}
