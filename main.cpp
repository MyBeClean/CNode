#include "scene/window/editor.h"

#include <iostream>

int main()
{
    CNode::EditorWindow editor;

    if (!editor.Initialize())
    {
        std::cerr << "CNode: Application startup failed.\n";
        return 1;
    }

    editor.Run();

    editor.Shutdown();

    return 0;
}