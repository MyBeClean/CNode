
#include "scene/window/editor.h"
#include "registration/registration_data.h"

#include <iostream>

int main()
{
    CNode::EditorWindow editor;

    if (!editor.Initialize())
    {
        std::cerr << "CNode: Application startup failed.\n";
        return 1;
    }

    CNode::Windows();

    editor.Run();

    editor.Shutdown();

    return 0;
}
