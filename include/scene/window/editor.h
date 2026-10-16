
#pragma once

#include <GLFW/glfw3.h>

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

        void Update();
        void Render();
    };
}
