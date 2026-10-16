
#pragma once

#include <GLFW/glfw3.h>
#include "scene/window/font.h"

namespace CNode
{
    class EditorWindow
    {
    public:
        EditorWindow();
        ~EditorWindow();

        bool Initialize();
        void Run();
        void Shutdown();

    private:
        GLFWwindow* window = nullptr;
        bool initialized = false;

        EditorFont editorFont;

        void Update();
        void Render();
    };
}
