#pragma once

#include <filesystem>

namespace CNode
{
    bool HasActiveProject();
    std::filesystem::path GetActiveProjectRoot();

    void DrawProjectManagerScreen();
    void DrawProjectMenu();
    void DrawProjectTab();
    void DrawProjectDialogs();

    void RequestCreateProject();
    void RequestOpenProject();
    void RequestDeleteProject();
    void RequestProjectSettings();
}