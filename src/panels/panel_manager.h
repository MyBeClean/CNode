
#pragma once

namespace CNode
{
    enum class PanelType
    {
        SceneEditor,
        NodeGraphEditor,
        Inspector,
        ProjectSettings,
        ObjectList
    };

    void AddPanel(
        PanelType type,
        bool floating = true
    );

    void DrawAllPanels();
}
