
#pragma once

namespace CNode
{
    enum class PanelType
    {
        SceneEditor,
        NodeGraphEditor,
        Inspector,
        ProjectSettings
    };

    void AddPanel(PanelType type, bool floating = true);
    void DrawAllPanels();
}
