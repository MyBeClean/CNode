#pragma once

#include <string>

namespace CNode
{
    void DrawFileTab();
    void DrawSceneDialogs();

    void NewScene();
    void OpenScene();
    bool SaveScene();

    std::string GetActiveSceneDisplayName();
}