
#pragma once

#include "scene_camera.h"

namespace CNode
{
    SceneEditor::Camera& GetSceneCamera();

    void DrawSceneEditor(
        const char* title,
        bool* open
    );
}
