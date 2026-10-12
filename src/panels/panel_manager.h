
#pragma once

namespace CNode
{
    enum class PanelType
    {
        SceneEditor,
        NodeGraphEditor,
        Inspector
    };

    void AddPanel(PanelType type, bool floating = true);
    void DrawAllPanels();
}
