#pragma once

#include <string>

namespace CNode
{
    void DrawFileTab();
    void DrawSceneDialogs();

    void NewScene();
    void OpenScene();
    bool SaveScene();

    void DrawSceneDocumentTabs();

    std::string GetActiveSceneDisplayName();
}