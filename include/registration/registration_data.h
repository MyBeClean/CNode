
#pragma once

#include "imgui.h"

#include <string>
#include <vector>

namespace CNode
{
    using WindowDrawFunction = void (*)(const char* title, bool* open);

    struct WindowType
    {
        std::string id;
        std::string name;
        WindowDrawFunction draw = nullptr;

        ImGuiDir defaultDockDirection = ImGuiDir_None;
    };

    struct WindowInstance
    {
        unsigned long long instanceID = 0;
        std::string typeID;
        std::string title;
        bool open = true;
    };

    // Mendaftarkan tipe window ke registry aktif.
    bool RegisterWindowType(
        const char* id,
        const char* name,
        WindowDrawFunction draw,
        ImGuiDir defaultDockDirection = ImGuiDir_None
    );

    // Mendaftarkan metadata dari attribute.
    bool RegisterWindowAttribute(
        const char* id,
        const char* name,
        WindowDrawFunction draw,
        ImGuiDir defaultDockDirection = ImGuiDir_None
    );

    // Memproses seluruh attribute yang sudah ditemukan.
    void RegisterWindowAttributes();

    WindowInstance* AddWindow(const char* typeID);

    const std::vector<WindowType>& GetRegisteredWindows();
    std::vector<WindowInstance>& GetWindowInstances();

    // Tetap menjadi entry point registry window.
    void Windows();
}
