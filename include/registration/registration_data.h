
#pragma once

#include <vector>

namespace CNode
{
    using WindowDrawFunction = void (*)(bool* open);

    struct WindowAttribute
    {
        const char* id;
        const char* name;
        bool defaultOpen;
        WindowDrawFunction draw;

        // Status runtime, terpisah dari defaultOpen.
        bool open = false;
    };

    bool RegisterWindow(const WindowAttribute& attribute);

    std::vector<WindowAttribute>& GetRegisteredWindows();

    void Windows();
}
